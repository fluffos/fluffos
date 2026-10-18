#ifndef FLUFFOS_VM_INTERNAL_HOOKS_H_
#define FLUFFOS_VM_INTERNAL_HOOKS_H_

#include <cstdint>

struct object_t;
struct program_t;
struct svalue_t;
struct funptr_t;
struct array_t;
struct mapping_t;
struct function_t;

/* Fast path: call sites test these bits before any hook call. When the mask
 * is zero the hot paths pay one predicted branch and nothing else.
 * Fine bits (16+) gate sub-work so an AFTER-only attachment does not walk
 * BEFORE/AROUND lists or take timestamps. */
enum HookMaskBits : uint32_t {
  HOOK_MASK_CALL_OTHER_MISS = 1u << 0,
  HOOK_MASK_OBJECT_CREATE = 1u << 1,
  HOOK_MASK_OBJECT_DESTRUCT = 1u << 2,
  HOOK_MASK_LPC_FUNC = 1u << 3, /* any `function` (LPC) attachment */
  HOOK_MASK_EFUN = 1u << 4,     /* any efun attachment */
  HOOK_MASK_NET_ACCEPT = 1u << 5,
  HOOK_MASK_NET_READ = 1u << 6,
  HOOK_MASK_NET_WRITE = 1u << 7,
  HOOK_MASK_NET_TELNET = 1u << 8,
  HOOK_MASK_NET_TLS = 1u << 9,
  HOOK_MASK_NET_RESOLVE = 1u << 10,
  HOOK_MASK_NET_WS_UPGRADE = 1u << 11,
  HOOK_MASK_APPLY_MISS = 1u << 12, /* apply() miss, non-call_other origins */
  /* Fine gates for the LPC `function` point only (efun uses HOOK_MASK_EFUN
   * for both before and after — one empty list walk is cheap vs. waking
   * the LPC call path). */
  HOOK_MASK_LPC_FUNC_ENTRY = 1u << 16,  /* BEFORE / DECIDE / count@before */
  HOOK_MASK_LPC_FUNC_AROUND = 1u << 17, /* AROUND */
  HOOK_MASK_LPC_FUNC_AFTER = 1u << 18,  /* AFTER / time / count@after */
  HOOK_MASK_TIME = 1u << 19,            /* need per-frame timestamps */
};

extern uint32_t g_hook_mask;

/* Non-zero while a handler is exempt or unused eval credit is pending.
 * interpret.cc tests this before calling hooks_eval_slowpath() so the
 * common outoftime path pays no call when hooks are idle. */
extern int g_hook_eval_active;

/* Kind / flag bits (must match src/include/hooks.h). */
constexpr int kHookBefore = 0;
constexpr int kHookDecide = 0x01;
constexpr int kHookAfter = 0x02;
constexpr int kHookAround = 0x04;
constexpr int kHookFailClosed = 0x10;
constexpr int kHookDeferred = 0x20;
constexpr int kHookOverrideNomask = 0x40;

enum class HookMissResult {
  NoOpinion,
  Answered,
};

/* Decide outcome for function / efun / net entry. */
enum class HookGateResult {
  NoOpinion, /* run the real call */
  Refuse,    /* skip the real call; optional refuse_msg */
  Answered,  /* skip the real call; *out is the result */
};

enum class HookRun : int {
  Ran = 0,
  Declined,
  Error,
  Skipped,
};

void hooks_init();
void hooks_clear();

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_hooks();
#endif

void hooks_for_each_suspended_stack(
    void (*value_fn)(svalue_t* start, svalue_t* end_inclusive, void* ctx),
    void (*frame_fn)(struct control_stack_t* cbase, struct control_stack_t* csp_inclusive, void* ctx),
    void* ctx);

bool hooks_suspended_frame_uses_prog(program_t* prog);
void hooks_remove_object_from_suspended_stacks(object_t* ob);

bool hooks_eval_slowpath();
void hooks_on_set_eval();
int64_t hooks_eval_credit();

/* True when a live handler may want to extend / restore the eval timer. */
inline bool hooks_eval_may_continue() {
  return g_hook_eval_active != 0 && hooks_eval_slowpath();
}

/* ---- join points ---- */

HookMissResult hook_dispatch_call_other_miss(object_t* named_target, const char* fn, int num_arg,
                                            const char* reason, svalue_t* out);

/* apply:miss — same reasons as call_other:miss, for every apply_low origin
 * except ORIGIN_CALL_OTHER (which has its own point). origin is
 * origin_to_name(). Handler arity includes origin as the last argument. */
HookMissResult hook_dispatch_apply_miss(object_t* named_target, const char* fn, int num_arg,
                                       const char* reason, const char* origin, svalue_t* out);

void hook_dispatch_object_create(object_t* ob);
void hook_dispatch_object_destruct(object_t* ob, const char* cause);

/* function — BEFORE/DECIDE before the body; AFTER after it returns.
 * Args for the call are on the stack (num_arg). origin is origin_to_name().
 * On Refuse/Answered the caller must not run the body; on Answered *out is
 * owned by the caller. AFTER sees the return value still on the stack. */
HookGateResult hook_dispatch_function_before(object_t* target, const char* fn, const char* origin,
                                             int num_arg, svalue_t* out);
void hook_dispatch_function_after(object_t* target, const char* fn, const char* origin, int num_arg);

/* AROUND for function: if any match, runs the highest-priority AROUND instead
 * of the body. The handler uses hook_proceed() to run the real call.
 * Returns true if an AROUND handled the call (result left on stack). */
bool hook_dispatch_function_around(object_t* target, const char* fn, const char* origin, int num_arg,
                                   program_t* prog, unsigned short address, function_t* funp);

/* efun:<name> — before the C efun runs. name is instrs[].name (no "efun:" prefix).
 * On return, *filter_ob_out (when non-null) is the first live object argument
 * used for object/target matching — pass it to hook_dispatch_efun_after, since
 * the efun consumes its args before AFTER runs. AFTER's `result` is the efun's
 * return value on the stack, or nullptr for a void efun (handlers then see 0). */
HookGateResult hook_dispatch_efun_before(const char* efun_name, int num_arg, svalue_t* out,
                                         object_t** filter_ob_out);
void hook_dispatch_efun_after(const char* efun_name, object_t* filter_ob, svalue_t* result);

/* net:accept — peer address string; false means refuse (caller closes fd). */
bool hook_dispatch_net_accept(const char* addr);

/* net:read / net:write — observe; DECIDE refuse drops the payload.
 * `data`/`len` is a raw byte span (may contain NULs). For net:write,
 * call hook_dispatch_net_write() before the transport handoff (BEFORE +
 * DECIDE); on success call hook_dispatch_net_write_after() after. */
bool hook_dispatch_net_read(object_t* user_ob, const char* data, int len);
bool hook_dispatch_net_write(object_t* user_ob, const char* data, int len);
void hook_dispatch_net_write_after(object_t* user_ob, const char* data, int len);

/* net:telnet — BEFORE on will/wont/do/dont/iac/sb; DECIDE refuse drops SB.
 * Returns false when a DECIDE handler refused (caller skips SB handling).
 * `payload`/`payload_len` is the SB body (may contain NULs); empty otherwise. */
bool hook_dispatch_net_telnet(object_t* user_ob, const char* event, const char* option,
                              const char* payload, int payload_len);

/* net:tls — AFTER observe of handshake outcome ("ok" / "error"). */
void hook_dispatch_net_tls(object_t* user_ob, int port, const char* version, const char* cipher,
                           const char* outcome);

/* net:resolve — AFTER observe of driver reverse-DNS completion. */
void hook_dispatch_net_resolve(const char* address, const char* name, object_t* user_ob);

bool hook_dispatch_net_ws_upgrade(const char* addr);

void hooks_owner_destructed(object_t* ob);
void hooks_revalidate_all();

/* Mark the current control-stack frame for function AFTER / "time" profiling.
 * Call after push_control_stack when the body will run (not AROUND/answered).
 * filter_target is the object used for object/target matching (named target
 * before the shadow walk); nullptr means use current_object at F_RETURN. */
void hooks_mark_frame_for_after(object_t* filter_target = nullptr);

/* AROUND support: mudlib efun hook_proceed(). */
bool hooks_around_active();
void f_hook_proceed();

int64_t hook_efun_attach(const char* point, svalue_t* action, mapping_t* spec, int flags);
void hook_efun_detach(int64_t id);
mapping_t* hook_efun_query(int64_t id);
array_t* hook_efun_list(object_t* ob /* nullable */);

#endif /* FLUFFOS_VM_INTERNAL_HOOKS_H_ */
