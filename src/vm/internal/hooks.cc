#include "base/std.h"

#include "vm/internal/hooks.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "base/internal/rc.h"
#include "vm/internal/apply.h"
#include "vm/internal/base/array.h"
#include "vm/internal/base/function.h"
#include "vm/internal/base/interpret.h"
#include "vm/internal/base/vm_stack.h"
#include "vm/internal/base/mapping.h"
#include "vm/internal/base/object.h"
#include "vm/internal/base/program.h"
#include "vm/internal/base/svalue.h"
#include "vm/internal/eval_limit.h"
#include "vm/internal/master.h"
#include "vm/internal/simulate.h"
#include "vm/internal/base/machine.h"
#include "base/internal/port.h"
#include "compiler/internal/keyword.h"
#include "compiler/internal/lexer.h"
#include "efuns.autogen.h"
#include "packages/core/heartbeat.h"
#include "thirdparty/scope_guard/scope_guard.hpp"
#include "applies_table.autogen.h"
#include "vm/internal/apply.h"
#include "vm/internal/base/interpret.h"
#ifdef __linux__
#include "vm/internal/posix_timers.h"
#endif

/* Kind bits (must match src/include/hooks.h). */
#ifndef HOOK_BEFORE
#define HOOK_BEFORE 0
#define HOOK_DECIDE 0x01
#define HOOK_AFTER 0x02
#define HOOK_AROUND 0x04
#define HOOK_FAIL_CLOSED 0x10
#define HOOK_DEFERRED 0x20
#define HOOK_OVERRIDE_NOMASK 0x40
#endif

uint32_t g_hook_mask = 0;
int g_hook_eval_active = 0;

/* Eval credit (lazy). Cleared by set_eval() when not inside a handler. */
int64_t g_hook_eval_credit = 0;
int64_t g_hook_allow_used = 0;
int g_hook_exempt = 0;

void refresh_eval_active() {
  g_hook_eval_active = (g_hook_exempt || g_hook_eval_credit > 0) ? 1 : 0;
}

namespace {

constexpr int kHookStackSize = 4096;
constexpr int kHookDepth = 64;
constexpr int kKindBits = HOOK_DECIDE | HOOK_AFTER | HOOK_AROUND; /* BEFORE is 0 */

svalue_t g_hook_stack_storage[10 + kHookStackSize + 10];
control_stack_t g_hook_cstack_storage[5 + kHookDepth + 5];

struct SuspendedStack {
  svalue_t *start = nullptr, *saved_sp = nullptr, *end = nullptr;
  control_stack_t *cbase = nullptr, *csp = nullptr, *clast = nullptr;
  SuspendedStack* next = nullptr;
};

SuspendedStack* g_suspended = nullptr;
bool g_on_hook_stack = false;
int g_hook_depth = 0;

std::chrono::steady_clock::time_point g_hook_t0;
int64_t g_hook_budget_us = 0;
int64_t g_hook_caller_R = -1;

enum class PointId : int {
  CallOtherMiss = 0,
  ApplyMiss,
  ObjectCreate,
  ObjectDestruct,
  Function,
  Efun,
  NetAccept,
  NetRead,
  NetWrite,
  NetTelnet,
  NetTls,
  NetResolve,
  NetWsUpgrade,
  Count,
};

/* Object-pin index for call_other:miss / apply:miss / function.
 * When every live attachment pins an `object` (no `target` string),
 * events on other objects skip dispatch entirely. Master pins also
 * cover clones (see object_filter_matches); their basenames live in
 * master_basenames so a clone event is not rejected. */
struct PinIndex {
  bool scan_all = false;
  std::unordered_set<object_t*> objects;
  std::unordered_set<std::string> master_basenames;
};
PinIndex g_pin_index[static_cast<int>(PointId::Count)];

const char* point_name(PointId p) {
  switch (p) {
    case PointId::CallOtherMiss:
      return "call_other:miss";
    case PointId::ApplyMiss:
      return "apply:miss";
    case PointId::ObjectCreate:
      return "object:create";
    case PointId::ObjectDestruct:
      return "object:destruct";
    case PointId::Function:
      return "function";
    case PointId::Efun:
      return "efun:*";
    case PointId::NetAccept:
      return "net:accept";
    case PointId::NetRead:
      return "net:read";
    case PointId::NetWrite:
      return "net:write";
    case PointId::NetTelnet:
      return "net:telnet";
    case PointId::NetTls:
      return "net:tls";
    case PointId::NetResolve:
      return "net:resolve";
    case PointId::NetWsUpgrade:
      return "net:ws_upgrade";
    default:
      return "?";
  }
}

/* Parse join-point name. For efun:<name>, writes the name (or "*" / empty for
 * wildcard) into efun_name_out when non-null; caller must free_string it. */
PointId parse_point(const char* name, const char** efun_name_out) {
  if (efun_name_out) {
    *efun_name_out = nullptr;
  }
  if (!name) {
    return PointId::Count;
  }
  if (strcmp(name, "call_other:miss") == 0) {
    return PointId::CallOtherMiss;
  }
  if (strcmp(name, "apply:miss") == 0) {
    return PointId::ApplyMiss;
  }
  if (strcmp(name, "object:create") == 0) {
    return PointId::ObjectCreate;
  }
  if (strcmp(name, "object:destruct") == 0) {
    return PointId::ObjectDestruct;
  }
  if (strcmp(name, "function") == 0) {
    return PointId::Function;
  }
  if (strncmp(name, "efun:", 5) == 0) {
    if (efun_name_out) {
      const char* rest = name + 5;
      *efun_name_out = make_shared_string(rest[0] ? rest : "*");
    }
    return PointId::Efun;
  }
  if (strcmp(name, "net:accept") == 0) {
    return PointId::NetAccept;
  }
  if (strcmp(name, "net:read") == 0) {
    return PointId::NetRead;
  }
  if (strcmp(name, "net:write") == 0) {
    return PointId::NetWrite;
  }
  if (strcmp(name, "net:telnet") == 0) {
    return PointId::NetTelnet;
  }
  if (strcmp(name, "net:tls") == 0) {
    return PointId::NetTls;
  }
  if (strcmp(name, "net:resolve") == 0) {
    return PointId::NetResolve;
  }
  if (strcmp(name, "net:ws_upgrade") == 0) {
    return PointId::NetWsUpgrade;
  }
  return PointId::Count;
}

uint32_t point_mask(PointId p) {
  switch (p) {
    case PointId::CallOtherMiss:
      return HOOK_MASK_CALL_OTHER_MISS;
    case PointId::ApplyMiss:
      return HOOK_MASK_APPLY_MISS;
    case PointId::ObjectCreate:
      return HOOK_MASK_OBJECT_CREATE;
    case PointId::ObjectDestruct:
      return HOOK_MASK_OBJECT_DESTRUCT;
    case PointId::Function:
      return HOOK_MASK_LPC_FUNC;
    case PointId::Efun:
      return HOOK_MASK_EFUN;
    case PointId::NetAccept:
      return HOOK_MASK_NET_ACCEPT;
    case PointId::NetRead:
      return HOOK_MASK_NET_READ;
    case PointId::NetWrite:
      return HOOK_MASK_NET_WRITE;
    case PointId::NetTelnet:
      return HOOK_MASK_NET_TELNET;
    case PointId::NetTls:
      return HOOK_MASK_NET_TLS;
    case PointId::NetResolve:
      return HOOK_MASK_NET_RESOLVE;
    case PointId::NetWsUpgrade:
      return HOOK_MASK_NET_WS_UPGRADE;
    default:
      return 0;
  }
}

/* Allowed kinds per point. Event points accept kind 0 as "the point's kind". */
bool kind_allowed(PointId p, int kind) {
  switch (p) {
    case PointId::CallOtherMiss:
    case PointId::ApplyMiss:
      return kind == HOOK_BEFORE || kind == HOOK_DECIDE;
    case PointId::ObjectCreate:
      return kind == HOOK_BEFORE /* 0 = after */ || kind == HOOK_AFTER;
    case PointId::ObjectDestruct:
      return kind == HOOK_BEFORE; /* HOOK_BEFORE is 0 = the point's kind */
    case PointId::Function:
      return kind == HOOK_BEFORE || kind == HOOK_DECIDE || kind == HOOK_AFTER ||
             kind == HOOK_AROUND;
    case PointId::Efun:
      return kind == HOOK_BEFORE || kind == HOOK_DECIDE || kind == HOOK_AFTER;
    case PointId::NetAccept:
    case PointId::NetWsUpgrade:
      return kind == HOOK_BEFORE || kind == HOOK_DECIDE;
    case PointId::NetRead:
      return kind == HOOK_BEFORE || kind == HOOK_DECIDE;
    case PointId::NetWrite:
      return kind == HOOK_AFTER || kind == HOOK_BEFORE || kind == HOOK_DECIDE;
    case PointId::NetTelnet:
      return kind == HOOK_BEFORE || kind == HOOK_DECIDE;
    case PointId::NetTls:
    case PointId::NetResolve:
      /* Observe-only; kind 0 means AFTER (normalize_kind). */
      return kind == HOOK_AFTER || kind == HOOK_BEFORE;
    default:
      return false;
  }
}

int kind_from_flags(int flags) {
  int bits = flags & kKindBits;
  if (bits == 0) {
    return HOOK_BEFORE;
  }
  /* Exactly one kind bit. */
  if ((bits & (bits - 1)) != 0) {
    return -1;
  }
  return bits;
}

int normalize_kind(PointId p, int flags) {
  int kind = kind_from_flags(flags);
  if (kind < 0) {
    return kind;
  }
  if (p == PointId::ObjectCreate && kind == HOOK_BEFORE) {
    return HOOK_AFTER; /* kind 0 means the point's kind */
  }
  if (p == PointId::ObjectDestruct && kind == 0) {
    return HOOK_BEFORE;
  }
  if ((p == PointId::NetTls || p == PointId::NetResolve) && kind == HOOK_BEFORE) {
    return HOOK_AFTER; /* kind 0 means AFTER for observe-only net points */
  }
  return kind;
}

struct CountRow {
  const char* caller_prog = nullptr; /* shared string, ref held */
  const char* target_prog = nullptr;
  const char* function = nullptr;
  int64_t count = 0;
  int64_t eval_us = 0;
};

struct Attachment {
  LPC_INT id = 0;
  PointId point = PointId::Count;
  const char* point_string = nullptr; /* shared; original attach name (efun:foo) */
  int flags = 0;
  int kind = 0;
  int priority = 0;
  int64_t expires = 0; /* 0 = never; wall seconds */
  object_t* owner = nullptr;
  funptr_t* fp = nullptr; /* null when action is count/time */
  bool is_count = false;
  bool is_time = false;
  const char* label = nullptr; /* shared string */
  /* Filters. Object filters hold refs. */
  object_t* filter_object = nullptr;
  object_t* filter_caller_object = nullptr;
  const char* filter_function = nullptr; /* shared */
  const char* filter_target = nullptr;   /* shared, canonical obname */
  const char* filter_caller = nullptr;
  const char* filter_caller_program = nullptr;
  const char* filter_origin = nullptr; /* shared; function / apply:miss */
  const char* exclude_caller_program = nullptr; /* shared; reject if matches */
  const char* efun_name = nullptr;              /* shared; PointId::Efun ("*" = all) */
  bool detached = false;
  int errors = 0;
  int64_t events = 0;
  int64_t skipped = 0;
  int64_t dropped = 0;
  int64_t eval_us = 0;
  int64_t last_fired = 0;
  std::vector<CountRow> rows;
};

/* Active AROUND: set while the highest-priority AROUND handler runs. */
struct AroundState {
  bool active = false;
  bool proceeded = false;
  object_t* target = nullptr;
  const char* fn = nullptr;
  const char* origin = nullptr;
  int num_arg = 0;
  program_t* prog = nullptr;
  unsigned short address = 0;
  function_t* funp = nullptr;
  int function_index_offset = 0;
  int variable_index_offset = 0;
  /* Original call args parked while the handler runs (moved, not copied). */
  std::vector<svalue_t> saved_args;
};
AroundState g_around;

/* Entry timestamps for FRAME_HOOKED frames ("time" action on AFTER). */
std::chrono::steady_clock::time_point g_hook_frame_t0[CFG_MAX_CALL_DEPTH + 8];

std::unordered_map<LPC_INT, Attachment*> g_attachments;
std::vector<Attachment*> g_by_point[static_cast<int>(PointId::Count)];
LPC_INT g_next_id = 1;

/* Code that keeps Attachment pointers across a handler call -- a dispatcher
 * walking its snapshot (BEFORE, then DECIDE), hook_call() itself -- runs in a
 * HookDispatchScope. A handler can detach any attachment: itself, another at
 * the same point, any whose owner it destructs. While a scope is open,
 * detach_attachment() unlinks and releases it as usual but parks the struct
 * here (still readable, `detached` set) for the outermost scope to delete. */
int g_hook_dispatches = 0;
std::vector<Attachment*> g_detached_attachments;

class HookDispatchScope {
 public:
  HookDispatchScope() { g_hook_dispatches++; }
  ~HookDispatchScope() {
    if (--g_hook_dispatches == 0) {
      for (Attachment* a : g_detached_attachments) {
        delete a;
      }
      g_detached_attachments.clear();
    }
  }
  HookDispatchScope(const HookDispatchScope&) = delete;
  HookDispatchScope& operator=(const HookDispatchScope&) = delete;
};

int64_t us_since(std::chrono::steady_clock::time_point t0) {
  return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t0)
      .count();
}

void raw_timer_set(int64_t us) {
#ifdef __linux__
  posix_eval_timer_set(us > 0 ? us : 1);
#endif
  outoftime = 0;
}

void rebuild_pin_index(PointId p) {
  PinIndex& idx = g_pin_index[static_cast<int>(p)];
  idx.scan_all = false;
  idx.objects.clear();
  idx.master_basenames.clear();
  auto& v = g_by_point[static_cast<int>(p)];
  for (Attachment* a : v) {
    if (a->detached) {
      continue;
    }
    /* target string (incl. prefix) or a missing object pin → full scan. */
    if (a->filter_target || !a->filter_object) {
      idx.scan_all = true;
      idx.objects.clear();
      idx.master_basenames.clear();
      return;
    }
    idx.objects.insert(a->filter_object);
    /* Master pins match clones by basename — index those names too. */
    if (!(a->filter_object->flags & O_CLONE) && a->filter_object->obname) {
      idx.master_basenames.emplace(a->filter_object->obname);
    }
  }
}

/* Strip a trailing "#N" clone suffix from obname into buf (NUL-terminated). */
void obname_without_clone(const char* obname, char* buf, size_t buflen) {
  if (!obname || buflen == 0) {
    if (buflen) {
      buf[0] = '\0';
    }
    return;
  }
  size_t n = strnlen(obname, buflen - 1);
  memcpy(buf, obname, n);
  buf[n] = '\0';
  char* hash = strchr(buf, '#');
  if (hash) {
    *hash = '\0';
  }
}

/* True when no live attachment for `p` can match `target` via object pins. */
bool pin_index_rejects(PointId p, object_t* target) {
  PinIndex const& idx = g_pin_index[static_cast<int>(p)];
  if (idx.scan_all) {
    return false;
  }
  if (!target) {
    return true;
  }
  if (idx.objects.find(target) != idx.objects.end()) {
    return false;
  }
  /* Clone of a pinned master? */
  if (!idx.master_basenames.empty() && target->obname) {
    char base[1024];
    obname_without_clone(target->obname, base, sizeof(base));
    if (idx.master_basenames.find(base) != idx.master_basenames.end()) {
      return false;
    }
  }
  return true;
}

void recompute_mask() {
  uint32_t m = 0;
  for (int i = 0; i < static_cast<int>(PointId::Count); i++) {
    auto& v = g_by_point[i];
    if (v.empty()) {
      continue;
    }
    PointId const p = static_cast<PointId>(i);
    m |= point_mask(p);
    if (p == PointId::Function) {
      for (Attachment* a : v) {
        if (a->detached) {
          continue;
        }
        if (a->kind == HOOK_AROUND) {
          m |= HOOK_MASK_LPC_FUNC_AROUND;
        } else if (a->kind == HOOK_AFTER || a->is_time) {
          m |= HOOK_MASK_LPC_FUNC_AFTER;
          if (a->is_time) {
            m |= HOOK_MASK_TIME;
          }
        } else {
          /* BEFORE, DECIDE, or count@before (kind 0 / HOOK_BEFORE). */
          m |= HOOK_MASK_LPC_FUNC_ENTRY;
        }
      }
    }
  }
  g_hook_mask = m;
  rebuild_pin_index(PointId::CallOtherMiss);
  rebuild_pin_index(PointId::ApplyMiss);
  rebuild_pin_index(PointId::Function);
}

void sort_point(PointId p) {
  auto& v = g_by_point[static_cast<int>(p)];
  /* Higher priority runs first; equal priority keeps attach order. */
  std::stable_sort(v.begin(), v.end(), [](Attachment* a, Attachment* b) {
    if (a->priority != b->priority) {
      return a->priority > b->priority;
    }
    return a->id < b->id;
  });
}

bool compiling_now() { return current_file != nullptr; }

bool funp_stale(funptr_t* f) {
  if (!f || !f->hdr.owner || (f->hdr.owner->flags & O_DESTRUCTED)) {
    return true;
  }
  if (!f->hdr.owner->prog) {
    return true;
  }
  return f->hdr.owner_gen != f->hdr.owner->prog_generation;
}

/* ---- simulate state save/restore (error_handler zeroes these) ---- */

struct SimulateSnap {
  object_t* restrict_destruct;
  int num_objects_this_thread;
};

SimulateSnap save_simulate() {
  return {hook_get_restrict_destruct(), hook_get_num_objects_this_thread()};
}
void restore_simulate(SimulateSnap s) {
  hook_set_restrict_destruct(s.restrict_destruct);
  hook_set_num_objects_this_thread(s.num_objects_this_thread);
}

/* ---- stack switch ---- */

void switch_to_hook_stack(SuspendedStack* ss, int n_args_already_on_main) {
  svalue_t* args = vm_sp() - n_args_already_on_main + 1;
  svalue_t* alt_start = &g_hook_stack_storage[10];
  for (int i = 0; i < n_args_already_on_main; i++) {
    alt_start[i] = args[i]; /* move, no ref change */
  }
  *ss = {start_of_stack, vm_sp() - n_args_already_on_main, end_of_stack, control_stack, csp,
         control_stack_last, g_suspended};
  g_suspended = ss;
  start_of_stack = alt_start;
  end_of_stack = alt_start + kHookStackSize;
  vm_sp() = alt_start + n_args_already_on_main - 1;
  control_stack = &g_hook_cstack_storage[5];
  control_stack_last = control_stack + kHookDepth - 1;
  csp = control_stack - 1;
  g_on_hook_stack = true;
}

void switch_back(SuspendedStack* ss) {
  start_of_stack = ss->start;
  end_of_stack = ss->end;
  vm_sp() = ss->saved_sp;
  control_stack = ss->cbase;
  control_stack_last = ss->clast;
  csp = ss->csp;
  g_suspended = ss->next;
  g_on_hook_stack = false;
}

/* ---- detach ---- */

void free_attachment_filters(Attachment* a) {
  if (a->filter_object) {
    free_object(&a->filter_object, "hook filter_object");
  }
  if (a->filter_caller_object) {
    free_object(&a->filter_caller_object, "hook filter_caller_object");
  }
  if (a->filter_function) {
    free_string(a->filter_function);
    a->filter_function = nullptr;
  }
  if (a->filter_target) {
    free_string(a->filter_target);
    a->filter_target = nullptr;
  }
  if (a->filter_caller) {
    free_string(a->filter_caller);
    a->filter_caller = nullptr;
  }
  if (a->filter_caller_program) {
    free_string(a->filter_caller_program);
    a->filter_caller_program = nullptr;
  }
  if (a->filter_origin) {
    free_string(a->filter_origin);
    a->filter_origin = nullptr;
  }
  if (a->exclude_caller_program) {
    free_string(a->exclude_caller_program);
    a->exclude_caller_program = nullptr;
  }
  if (a->efun_name) {
    free_string(a->efun_name);
    a->efun_name = nullptr;
  }
  if (a->point_string) {
    free_string(a->point_string);
    a->point_string = nullptr;
  }
  if (a->label) {
    free_string(a->label);
    a->label = nullptr;
  }
  for (auto& r : a->rows) {
    if (r.caller_prog) {
      free_string(r.caller_prog);
    }
    if (r.target_prog) {
      free_string(r.target_prog);
    }
    if (r.function) {
      free_string(r.function);
    }
  }
  a->rows.clear();
}

void detach_attachment(Attachment* a) {
  if (!a || a->detached) {
    return;
  }
  a->detached = true;
  auto& v = g_by_point[static_cast<int>(a->point)];
  v.erase(std::remove(v.begin(), v.end(), a), v.end());
  g_attachments.erase(a->id);
  recompute_mask();
  if (a->fp) {
    free_funp(a->fp);
    a->fp = nullptr;
  }
  if (a->owner) {
    free_object(&a->owner, "hook owner");
  }
  free_attachment_filters(a);
  if (g_hook_dispatches > 0) {
    g_detached_attachments.push_back(a); /* see HookDispatchScope */
    return;
  }
  delete a;
}

/* ---- matching ---- */

/* Name patterns for obname / function / efun / program filters:
 *   "*"       — match any non-null name
 *   "foo*"    — prefix match (single trailing wildcard)
 *   "foo/"    — prefix match (path convention; '/' included in the prefix)
 *   "foo"     — exact match
 * A '*' anywhere other than alone or as the final character is rejected at
 * attach time (see validate_name_pattern). Bound for filter/name scans —
 * attach rejects longer patterns. */
constexpr size_t kMaxHookNamePattern = 1023;

bool name_matches(const char* filter, const char* name) {
  if (!filter) {
    return true;
  }
  if (!name) {
    return false;
  }
  if (filter[0] == '*' && filter[1] == '\0') {
    return true;
  }
  size_t flen = strnlen(filter, kMaxHookNamePattern + 1);
  if (flen > kMaxHookNamePattern) {
    return false;
  }
  if (flen && filter[flen - 1] == '*') {
    return strncmp(filter, name, flen - 1) == 0;
  }
  if (flen && filter[flen - 1] == '/') {
    return strncmp(filter, name, flen) == 0;
  }
  return strcmp(filter, name) == 0;
}

/* Reject embedded / multiple '*'. Empty "foo*" prefix is allowed. */
void validate_name_pattern(const char* filter, const char* what) {
  if (!filter) {
    return;
  }
  if (strnlen(filter, kMaxHookNamePattern + 1) > kMaxHookNamePattern) {
    error("hook_attach: %s pattern too long.\n", what);
  }
  const char* star = strchr(filter, '*');
  if (!star) {
    return;
  }
  if (filter[0] == '*' && filter[1] == '\0') {
    return;
  }
  if (star[1] != '\0' || star != strrchr(filter, '*')) {
    error("hook_attach: %s pattern allows only \"*\" or a single trailing '*'.\n", what);
  }
}

/* Match filter against an object name, ignoring a trailing "#clone" suffix. */
bool object_name_matches(const char* filter, const char* obname) {
  if (!filter) {
    return true;
  }
  if (!obname) {
    return false;
  }
  char buf[1024];
  obname_without_clone(obname, buf, sizeof(buf));
  return name_matches(filter, buf);
}

/* `object` pin match: exact pointer, or a master pin covering its clones
 * (same basename rule as `target:`). A clone pin stays instance-exact so
 * per-object status effects do not spill to sibling clones. */
bool object_filter_matches(object_t* filter, object_t* ob) {
  if (!filter) {
    return true;
  }
  if (!ob) {
    return false;
  }
  if (filter == ob) {
    return true;
  }
  if ((filter->flags & O_CLONE) || !filter->obname) {
    return false;
  }
  return object_name_matches(filter->obname, ob->obname);
}

/* Resolve efun aliases (clone_object → _new) to the instrs[] primary name.
 * Patterns ("*", "foo*") are left unchanged. */
const char* canonical_efun_name(const char* name) {
  if (!name || !name[0] || strchr(name, '*') != nullptr) {
    return name;
  }
  for (int i = 0; i < size_of_predefs; i++) {
    if (predefs[i].word && strcmp(predefs[i].word, name) == 0) {
      unsigned short tok = predefs[i].token;
      if (tok < MAX_INSTRS && instrs[tok].name) {
        return instrs[tok].name;
      }
      return name;
    }
  }
  return name; /* unknown — keep as typed; match will simply never fire */
}

/* Match an efun filter against the dispatch (primary) name. Exact filters are
 * canonicalized at attach. Patterns also match when any alias of this
 * dispatch efun matches the pattern (so efun:clone* hits _new). */
bool efun_name_matches(const char* filter, const char* dispatch_name) {
  if (!filter) {
    return true;
  }
  if (name_matches(filter, dispatch_name)) {
    return true;
  }
  if (!dispatch_name || !strchr(filter, '*')) {
    return false;
  }
  for (int i = 0; i < size_of_predefs; i++) {
    if (!predefs[i].word) {
      continue;
    }
    unsigned short tok = predefs[i].token;
    if (tok >= MAX_INSTRS || !instrs[tok].name) {
      continue;
    }
    if (strcmp(instrs[tok].name, dispatch_name) != 0) {
      continue;
    }
    if (name_matches(filter, predefs[i].word)) {
      return true;
    }
  }
  return false;
}

bool attachment_expired(Attachment* a) {
  if (!a->expires) {
    return false;
  }
  return get_current_time() >= a->expires;
}

bool attachment_matches_miss(Attachment* a, object_t* named_target, const char* fn) {
  if (attachment_expired(a)) {
    detach_attachment(a);
    return false;
  }
  if (!object_filter_matches(a->filter_object, named_target)) {
    return false;
  }
  if (a->filter_target &&
      !object_name_matches(a->filter_target, named_target ? named_target->obname : nullptr)) {
    return false;
  }
  if (a->filter_function && !name_matches(a->filter_function, fn)) {
    return false;
  }
  if (a->filter_caller_object && a->filter_caller_object != current_object) {
    return false;
  }
  if (a->filter_caller &&
      !object_name_matches(a->filter_caller, current_object ? current_object->obname : nullptr)) {
    return false;
  }
  if (a->filter_caller_program) {
    const char* prog =
        (current_object && current_object->prog) ? current_object->prog->filename : nullptr;
    if (!name_matches(a->filter_caller_program, prog)) {
      return false;
    }
  }
  if (a->exclude_caller_program) {
    const char* prog =
        (current_object && current_object->prog) ? current_object->prog->filename : nullptr;
    if (name_matches(a->exclude_caller_program, prog)) {
      return false;
    }
  }
  return true;
}

bool attachment_matches_object(Attachment* a, object_t* ob) {
  if (attachment_expired(a)) {
    detach_attachment(a);
    return false;
  }
  if (!object_filter_matches(a->filter_object, ob)) {
    return false;
  }
  if (a->filter_target && !object_name_matches(a->filter_target, ob ? ob->obname : nullptr)) {
    return false;
  }
  return true;
}

bool attachment_matches_call(Attachment* a, object_t* target, const char* fn, const char* origin) {
  if (!attachment_matches_miss(a, target, fn)) {
    return false;
  }
  if (a->filter_origin && (!origin || strcmp(a->filter_origin, origin) != 0)) {
    return false;
  }
  return true;
}

bool attachment_matches_efun(Attachment* a, const char* efun_name, object_t* first_obj_arg) {
  if (attachment_expired(a)) {
    detach_attachment(a);
    return false;
  }
  if (a->efun_name && !efun_name_matches(a->efun_name, efun_name)) {
    return false;
  }
  if (!object_filter_matches(a->filter_object, first_obj_arg)) {
    return false;
  }
  if (a->filter_target &&
      !object_name_matches(a->filter_target, first_obj_arg ? first_obj_arg->obname : nullptr)) {
    return false;
  }
  if (a->filter_caller_object && a->filter_caller_object != current_object) {
    return false;
  }
  if (a->filter_caller &&
      !object_name_matches(a->filter_caller, current_object ? current_object->obname : nullptr)) {
    return false;
  }
  if (a->filter_caller_program) {
    const char* prog =
        (current_object && current_object->prog) ? current_object->prog->filename : nullptr;
    if (!name_matches(a->filter_caller_program, prog)) {
      return false;
    }
  }
  if (a->exclude_caller_program) {
    const char* prog =
        (current_object && current_object->prog) ? current_object->prog->filename : nullptr;
    if (name_matches(a->exclude_caller_program, prog)) {
      return false;
    }
  }
  return true;
}

object_t* first_object_arg(svalue_t* args, int n) {
  for (int i = 0; i < n; i++) {
    if (args[i].type == T_OBJECT && !(args[i].u.ob->flags & O_DESTRUCTED)) {
      return args[i].u.ob;
    }
  }
  return nullptr;
}

array_t* copy_args_one_level(svalue_t* args, int n);

/* Push (id, actor, caller, target, fn, args). Returns count pushed. */
int push_call_args(Attachment* a, object_t* target, const char* fn, svalue_t* call_args,
                   int n_call_args) {
  push_number(a->id);
  if (command_giver && !(command_giver->flags & O_DESTRUCTED)) {
    push_object(command_giver);
  } else {
    push_number(0);
  }
  if (current_object && !(current_object->flags & O_DESTRUCTED)) {
    push_object(current_object);
  } else {
    push_number(0);
  }
  if (target && !(target->flags & O_DESTRUCTED)) {
    push_object(target);
  } else {
    push_number(0);
  }
  copy_and_push_string(fn ? fn : "");
  array_t* args_copy = copy_args_one_level(call_args, n_call_args);
  push_refed_array(args_copy);
  return 6;
}

/* Interpret a DECIDE handler result into a gate outcome. */
HookGateResult decide_result(svalue_t* result, HookRun run, int flags, svalue_t* out,
                            bool allow_answer) {
  if (run == HookRun::Skipped || run == HookRun::Error || run == HookRun::Declined) {
    if (flags & HOOK_FAIL_CLOSED) {
      if (out) {
        *out = const0;
      }
      return HookGateResult::Refuse;
    }
    return HookGateResult::NoOpinion;
  }
  if (run != HookRun::Ran) {
    return HookGateResult::NoOpinion;
  }
  /* ({ value }) => answer */
  if (allow_answer && result->type == T_ARRAY && result->u.arr->size == 1) {
    if (out) {
      assign_svalue_no_free(out, &result->u.arr->item[0]);
    }
    return HookGateResult::Answered;
  }
  /* string => refuse with message */
  if (result->type == T_STRING) {
    if (out) {
      assign_svalue_no_free(out, result);
    }
    return HookGateResult::Refuse;
  }
  /* 1 => refuse */
  if (result->type == T_NUMBER && result->u.number == 1) {
    if (out) {
      *out = const0;
    }
    return HookGateResult::Refuse;
  }
  /* 0 / other => no opinion */
  return HookGateResult::NoOpinion;
}

/* ---- one-level arg copy ---- */

array_t* copy_args_one_level(svalue_t* args, int n) {
  array_t* arr = allocate_empty_array(n);
  for (int i = 0; i < n; i++) {
    svalue_t* src = &args[i];
    if (src->type == T_ARRAY) {
      arr->item[i].type = T_ARRAY;
      arr->item[i].u.arr = copy_array(src->u.arr);
    } else if (src->type == T_MAPPING) {
      /* load_mapping_from_aggregate-style shallow: copy_mapping not exported;
       * bump ref (observers must not mutate shared nodes they didn't create —
       * documented as one-level; nested compounds stay shared). */
      assign_svalue_no_free(&arr->item[i], src);
    } else if (src->type == T_BUFFER) {
      assign_svalue_no_free(&arr->item[i], src);
    } else if (src->type == T_CLASS) {
      arr->item[i].type = T_CLASS;
      arr->item[i].u.arr = copy_array(src->u.arr);
    } else {
      assign_svalue_no_free(&arr->item[i], src);
    }
  }
  return arr;
}

/* ---- contained call ---- */

HookRun hook_call(Attachment* a, int n_args_on_stack, svalue_t* out) {
  /* Callers push handler args then call us; the success path moves them onto
   * the alt stack (switch_to_hook_stack) and call_function_pointer consumes
   * them. Every early return must pop the same way, or the args (and every
   * ref they hold) stay on the VM stack — Debug check_memory() reports Bad
   * ref count after the next test that leaves those slots live. */
  auto pop_args = [&]() {
    if (n_args_on_stack > 0) {
      pop_n_elems(n_args_on_stack);
    }
  };
  /* The handler may detach `a` (and so may the stale-owner paths below);
   * it is still read on the way out. */
  HookDispatchScope const scope;
  if (a->detached) {
    pop_args();
    return HookRun::Skipped;
  }
  if (a->is_count || a->is_time) {
    pop_args();
    return HookRun::Ran; /* C-side actions handled by caller */
  }
  funptr_t* f = a->fp;
  if (!f || !f->hdr.owner || (f->hdr.owner->flags & O_DESTRUCTED)) {
    detach_attachment(a);
    pop_args();
    return HookRun::Skipped;
  }
  if (funp_stale(f)) {
    detach_attachment(a);
    pop_args();
    return HookRun::Skipped;
  }
  if (compiling_now() || (g_on_hook_stack && g_hook_depth >= kHookDepth - 4)) {
    a->skipped++;
    pop_args();
    return HookRun::Skipped;
  }

  auto const t0 = std::chrono::steady_clock::now();
  bool const outermost = !g_hook_depth++;
  object_t* ci = current_interactive;
  object_t* hb = g_current_heartbeat_obj;
  if (ci) {
    add_ref(ci, "hook");
  }
  if (hb) {
    add_ref(hb, "hook");
  }
  auto sim = save_simulate();
  f->hdr.ref++;

  SuspendedStack ss{};
  if (outermost) {
    switch_to_hook_stack(&ss, n_args_on_stack);
  }
  /* else: already on hook stack; args already placed by nested caller */

  save_command_giver(nullptr);
  current_interactive = nullptr;
  g_current_heartbeat_obj = nullptr;

  int64_t budget = CONFIG_INT(__RC_HOOK_EVAL_COST__);
  if (budget <= 0) {
    budget = CONFIG_INT(__MAX_EVAL_COST__);
  }
  bool const exempt = outermost;
  if (exempt) {
    g_hook_exempt++;
    refresh_eval_active();
    g_hook_t0 = t0;
    g_hook_budget_us = budget;
    g_hook_caller_R = -1;
  }

  error_context_t econ;
  save_context(&econ);
  econ.save_sp -= n_args_on_stack;
  svalue_t* ret = nullptr;
  try {
    ret = call_function_pointer(f, n_args_on_stack);
  } catch (const char*) {
    restore_context(&econ);
    ret = nullptr;
  }
  pop_context(&econ);

  if (outermost) {
    switch_back(&ss);
  }

  restore_command_giver();
  current_interactive = (ci && !(ci->flags & O_DESTRUCTED)) ? ci : nullptr;
  g_current_heartbeat_obj = (hb && !(hb->flags & O_DESTRUCTED)) ? hb : nullptr;
  if (ci) {
    free_object(&ci, "hook");
  }
  if (hb) {
    free_object(&hb, "hook");
  }
  restore_simulate(sim);
  /* Arity is a call_function_pointer parameter — no st_num_arg to restore. */

  if (exempt) {
    int64_t const h = us_since(t0);
    g_hook_exempt--;
    a->eval_us += h;
    int64_t uncharged = h;
    int64_t const left = budget - g_hook_allow_used;
    uncharged = left > 0 ? (h < left ? h : left) : 0;
    g_hook_allow_used += uncharged;
    if (g_hook_caller_R >= 0) {
      int64_t const remaining = g_hook_caller_R - (h - uncharged);
      if (remaining > 0) {
        raw_timer_set(remaining);
      } else {
        outoftime = 1;
      }
      refresh_eval_active();
    } else {
      g_hook_eval_credit += uncharged;
      refresh_eval_active();
    }
  }

  g_hook_depth--;
  free_funp(f);

  if (!ret) {
    a->errors++;
    too_deep_error = 0;
    max_eval_error = 0;
    outoftime = 0;
    int max_err = CONFIG_INT(__RC_HOOK_MAX_ERRORS__);
    if (max_err <= 0) {
      max_err = 10;
    }
    if (!(a->flags & HOOK_FAIL_CLOSED) && a->errors >= max_err) {
      detach_attachment(a);
    }
    return HookRun::Error;
  }
  if (ret->type == T_PROMISE || a->detached) {
    return HookRun::Declined;
  }
  if (out) {
    /* ret is &apply_ret_value (global); copy with bump — do not steal. */
    assign_svalue_no_free(out, ret);
  }
  a->events++;
  a->last_fired = get_current_time();
  return HookRun::Ran;
}

/* Push standard positional args for a miss handler onto the (current) stack.
 * When origin is non-null (apply:miss), it is appended. Returns argc. */
int push_miss_args(Attachment* a, object_t* named_target, const char* fn, svalue_t* call_args,
                   int n_call_args, const char* reason, const char* origin = nullptr) {
  push_number(a->id);
  if (command_giver && !(command_giver->flags & O_DESTRUCTED)) {
    push_object(command_giver);
  } else {
    push_number(0);
  }
  if (current_object && !(current_object->flags & O_DESTRUCTED)) {
    push_object(current_object);
  } else {
    push_number(0);
  }
  if (named_target && !(named_target->flags & O_DESTRUCTED)) {
    push_object(named_target);
  } else {
    push_number(0);
  }
  copy_and_push_string(fn ? fn : "");
  array_t* args_copy = copy_args_one_level(call_args, n_call_args);
  push_refed_array(args_copy);
  copy_and_push_string(reason ? reason : "undefined");
  if (origin) {
    copy_and_push_string(origin);
    return 8; /* id + 7 */
  }
  return 7; /* id + 6 */
}

void bump_count(Attachment* a, const char* caller_prog, const char* target_prog, const char* fn,
                int64_t add_us = 0) {
  int max_rows = CONFIG_INT(__RC_HOOK_MAX_ROWS__);
  if (max_rows <= 0) {
    max_rows = 10000;
  }
  for (auto& r : a->rows) {
    if (r.function == fn || (r.function && fn && strcmp(r.function, fn) == 0)) {
      if ((!r.caller_prog && !caller_prog) ||
          (r.caller_prog && caller_prog && strcmp(r.caller_prog, caller_prog) == 0)) {
        if ((!r.target_prog && !target_prog) ||
            (r.target_prog && target_prog && strcmp(r.target_prog, target_prog) == 0)) {
          r.count++;
          r.eval_us += add_us;
          a->events++;
          a->eval_us += add_us;
          return;
        }
      }
    }
  }
  if (static_cast<int>(a->rows.size()) >= max_rows) {
    a->dropped++;
    return;
  }
  CountRow r;
  r.caller_prog = caller_prog ? make_shared_string(caller_prog) : nullptr;
  r.target_prog = target_prog ? make_shared_string(target_prog) : nullptr;
  r.function = fn ? make_shared_string(fn) : nullptr;
  r.count = 1;
  r.eval_us = add_us;
  a->rows.push_back(r);
  a->events++;
  a->eval_us += add_us;
}

/* Snapshot the live list so attach/detach inside a handler only affects the
 * next event. Its dispatch scope keeps every entry readable for as long as
 * the dispatcher walks it: one a handler detached reads as `detached`. */
class HookSnapshot {
 public:
  explicit HookSnapshot(const std::vector<Attachment*>& live) : list_(live) {}

  std::vector<Attachment*>::const_iterator begin() const { return list_.begin(); }
  std::vector<Attachment*>::const_iterator end() const { return list_.end(); }

 private:
  HookDispatchScope scope_;
  std::vector<Attachment*> list_;
};

HookSnapshot snapshot_point(PointId p) { return HookSnapshot(g_by_point[static_cast<int>(p)]); }

/* ---- authorization ---- */

int compute_tier(PointId p, int kind, int flags, object_t* who, funptr_t* handler_fp,
                 object_t* filter_object, object_t* filter_caller_object, bool has_scope_key) {
  bool handler_is_who = handler_fp && handler_fp->hdr.owner == who;
  bool self_pinned = (filter_object == who || filter_caller_object == who) && handler_is_who &&
                     !(flags & HOOK_FAIL_CLOSED);
  if (self_pinned) {
    return 1;
  }
  bool observe = (kind == HOOK_BEFORE || kind == HOOK_AFTER);
  bool observe_point = (p == PointId::CallOtherMiss || p == PointId::ApplyMiss ||
                        p == PointId::ObjectCreate || p == PointId::ObjectDestruct ||
                        p == PointId::Function || p == PointId::Efun);
  if (observe && handler_is_who && observe_point && has_scope_key) {
    return 2;
  }
  return 3;
}

int ask_valid_hook(object_t* who, PointId p, svalue_t* action, mapping_t* spec_copy, int flags,
                   int needed_tier) {
  if (!master_ob) {
    return 0;
  }
  push_object(who);
  copy_and_push_string(point_name(p));
  push_svalue(action);
  push_refed_mapping(spec_copy); /* master gets a fresh copy; we hold one too via push */
  /* push_refed_mapping consumes? Check — usually push_mapping bumps. Use assign. */
  push_number(flags);
  push_number(needed_tier);
  svalue_t* ret = safe_apply_master_ob(APPLY_VALID_HOOK, 6);
  if (!ret || ret == (svalue_t*)-1) {
    return 0;
  }
  if (ret->type == T_PROMISE) {
    return 0;
  }
  if (ret->type != T_NUMBER) {
    return 0;
  }
  return static_cast<int>(ret->u.number);
}

const char* read_string_key(mapping_t* m, const char* key) {
  svalue_t k{};
  k.type = T_STRING;
  k.subtype = STRING_CONSTANT;
  k.u.string = key;
  svalue_t* v = find_in_mapping(m, k);
  if (!v || v->type != T_STRING) {
    return nullptr;
  }
  return v->u.string;
}

/* `object` / `caller_object` accept a live object or a name pattern string. */
void read_object_or_string_key(mapping_t* m, const char* key, object_t** out_ob,
                              const char** out_str) {
  *out_ob = nullptr;
  *out_str = nullptr;
  svalue_t k{};
  k.type = T_STRING;
  k.subtype = STRING_CONSTANT;
  k.u.string = key;
  svalue_t* v = find_in_mapping(m, k);
  if (!v) {
    return;
  }
  if (v->type == T_OBJECT) {
    if (!(v->u.ob->flags & O_DESTRUCTED)) {
      *out_ob = v->u.ob;
    }
    return;
  }
  if (v->type == T_STRING) {
    *out_str = v->u.string;
  }
}

int read_number_key(mapping_t* m, const char* key, int def) {
  svalue_t k{};
  k.type = T_STRING;
  k.subtype = STRING_CONSTANT;
  k.u.string = key;
  svalue_t* v = find_in_mapping(m, k);
  if (!v || v->type != T_NUMBER) {
    return def;
  }
  return static_cast<int>(v->u.number);
}

/* Canonicalize an object-name filter. Patterns ("*", "foo*", "foo/") keep
 * their suffix; the path body is normalized like filename_to_obname. */
const char* canonicalize_name(const char* in) {
  if (!in) {
    return nullptr;
  }
  if (in[0] == '*' && in[1] == '\0') {
    return make_shared_string("*");
  }
  size_t len = strnlen(in, kMaxHookNamePattern + 1);
  if (len > kMaxHookNamePattern) {
    error("hook_attach: name pattern too long.\n");
  }
  bool star_suffix = len > 0 && in[len - 1] == '*';
  bool slash_suffix = len > 0 && in[len - 1] == '/';
  char body[1024];
  size_t body_len = len;
  if (star_suffix || slash_suffix) {
    body_len = len - 1;
  }
  memcpy(body, in, body_len);
  body[body_len] = '\0';

  char buf[1024];
  if (body[0] == '\0') {
    /* "*" already handled; bare "*"/" → keep as pattern suffix only. */
    buf[0] = '\0';
  } else if (!filename_to_obname(body, buf, sizeof(buf))) {
    size_t n = body_len;
    if (n >= sizeof(buf)) {
      n = sizeof(buf) - 1;
    }
    memcpy(buf, body, n);
    buf[n] = '\0';
  } else {
    char* hash = strchr(buf, '#');
    if (hash) {
      *hash = '\0';
    }
  }
  /* Cap at sizeof-1 so blen+1 is always a valid index when we append. */
  size_t blen = strnlen(buf, sizeof(buf) - 1);
  if (star_suffix || slash_suffix) {
    if (blen < sizeof(buf) - 1) {
      buf[blen] = star_suffix ? '*' : '/';
      buf[blen + 1] = '\0';
    } else {
      error("hook_attach: name pattern too long.\n");
    }
  }
  return make_shared_string(buf);
}

mapping_t* attachment_to_mapping(Attachment* a, bool with_rows) {
  mapping_t* m = allocate_mapping(16);
  add_mapping_pair(m, "id", a->id);
  if (a->point_string) {
    add_mapping_shared_string(m, "point", const_cast<char*>(a->point_string));
  } else {
    add_mapping_string(m, "point", point_name(a->point));
  }
  if (a->owner && !(a->owner->flags & O_DESTRUCTED)) {
    add_mapping_object(m, "owner", a->owner);
  }
  if (a->label) {
    add_mapping_shared_string(m, "label", const_cast<char*>(a->label));
  }
  add_mapping_pair(m, "flags", a->flags);
  add_mapping_pair(m, "priority", a->priority);
  add_mapping_pair(m, "expires", a->expires);
  add_mapping_pair(m, "events", a->events);
  add_mapping_pair(m, "errors", a->errors);
  add_mapping_pair(m, "skipped", a->skipped);
  add_mapping_pair(m, "dropped", a->dropped);
  add_mapping_pair(m, "eval_us", a->eval_us);
  add_mapping_pair(m, "last_fired", a->last_fired);
  if (with_rows && (a->is_count || a->is_time)) {
    array_t* rows = allocate_empty_array(static_cast<int>(a->rows.size()));
    for (size_t i = 0; i < a->rows.size(); i++) {
      mapping_t* rm = allocate_mapping(5);
      if (a->rows[i].caller_prog) {
        add_mapping_shared_string(rm, "caller_program", const_cast<char*>(a->rows[i].caller_prog));
      }
      if (a->rows[i].target_prog) {
        add_mapping_shared_string(rm, "target_program", const_cast<char*>(a->rows[i].target_prog));
      }
      if (a->rows[i].function) {
        add_mapping_shared_string(rm, "function", const_cast<char*>(a->rows[i].function));
      }
      add_mapping_pair(rm, "count", a->rows[i].count);
      add_mapping_pair(rm, "eval_us", a->rows[i].eval_us);
      rows->item[i].type = T_MAPPING;
      rows->item[i].u.map = rm;
    }
    add_mapping_array(m, "rows", rows);
    free_array(rows); /* add_mapping_array bumps */
  }
  return m;
}

}  // namespace

/* ---- public API ---- */

void hooks_init() {
  g_hook_mask = 0;
  g_hook_eval_active = 0;
}

void hooks_clear() {
  std::vector<Attachment*> all;
  all.reserve(g_attachments.size());
  for (auto& kv : g_attachments) {
    all.push_back(kv.second);
  }
  g_attachments.clear();
  for (auto& v : g_by_point) {
    v.clear();
  }
  g_hook_mask = 0;
  for (int i = 0; i < static_cast<int>(PointId::Count); i++) {
    g_pin_index[i].scan_all = false;
    g_pin_index[i].objects.clear();
    g_pin_index[i].master_basenames.clear();
  }
  g_hook_eval_active = 0;
  g_hook_eval_credit = 0;
  g_hook_exempt = 0;
  for (Attachment* a : all) {
    if (a->fp) {
      free_funp(a->fp);
    }
    if (a->owner) {
      free_object(&a->owner, "hooks_clear");
    }
    free_attachment_filters(a);
    delete a;
  }
}

bool hooks_eval_slowpath() {
  if (g_hook_exempt) {
    int64_t const el = us_since(g_hook_t0);
    if (g_hook_caller_R < 0) {
      g_hook_caller_R = el;
    }
    if (el >= g_hook_budget_us) {
      return false;
    }
    raw_timer_set(g_hook_budget_us - el);
    return true;
  }
  if (g_hook_eval_credit > 0) {
    int64_t const c = g_hook_eval_credit;
    g_hook_eval_credit = 0;
    refresh_eval_active();
    raw_timer_set(c);
    return true;
  }
  return false;
}

void hooks_for_each_suspended_stack(
    void (*value_fn)(svalue_t* start, svalue_t* end_inclusive, void* ctx),
    void (*frame_fn)(control_stack_t* cbase, control_stack_t* csp_inclusive, void* ctx), void* ctx) {
  for (SuspendedStack* s = g_suspended; s; s = s->next) {
    if (value_fn) {
      value_fn(s->start, s->saved_sp, ctx);
    }
    if (frame_fn) {
      frame_fn(s->cbase, s->csp, ctx);
    }
  }
}

bool hooks_suspended_frame_uses_prog(program_t* prog) {
  for (SuspendedStack* s = g_suspended; s; s = s->next) {
    for (control_stack_t* f = s->cbase; f <= s->csp; f++) {
      if ((f->ob && f->ob->prog == prog) || f->prog == prog) {
        return true;
      }
    }
  }
  return false;
}

void hooks_remove_object_from_suspended_stacks(object_t* ob) {
  for (SuspendedStack* s = g_suspended; s; s = s->next) {
    for (svalue_t* svp = s->start; svp <= s->saved_sp; svp++) {
      if (svp->type == T_OBJECT && svp->u.ob == ob) {
        free_object(&svp->u.ob, "remove_object_from_stack (suspended)");
        svp->type = T_NUMBER;
        svp->u.number = 0;
      }
    }
  }
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_hooks() {
  for (auto& kv : g_attachments) {
    Attachment* a = kv.second;
    if (a->owner) {
      a->owner->extra_ref++;
    }
    if (a->fp) {
      a->fp->hdr.extra_ref++;
      if (a->fp->hdr.owner) {
        a->fp->hdr.owner->extra_ref++;
      }
      if (a->fp->hdr.args) {
        a->fp->hdr.args->extra_ref++;
      }
    }
    if (a->filter_object) {
      a->filter_object->extra_ref++;
    }
    if (a->filter_caller_object) {
      a->filter_caller_object->extra_ref++;
    }
    if (a->label) {
      EXTRA_REF(BLOCK(a->label))++;
    }
    if (a->filter_function) {
      EXTRA_REF(BLOCK(a->filter_function))++;
    }
    if (a->filter_target) {
      EXTRA_REF(BLOCK(a->filter_target))++;
    }
    if (a->filter_caller) {
      EXTRA_REF(BLOCK(a->filter_caller))++;
    }
    if (a->filter_caller_program) {
      EXTRA_REF(BLOCK(a->filter_caller_program))++;
    }
    if (a->filter_origin) {
      EXTRA_REF(BLOCK(a->filter_origin))++;
    }
    if (a->exclude_caller_program) {
      EXTRA_REF(BLOCK(a->exclude_caller_program))++;
    }
    if (a->efun_name) {
      EXTRA_REF(BLOCK(a->efun_name))++;
    }
    if (a->point_string) {
      EXTRA_REF(BLOCK(a->point_string))++;
    }
    for (auto& r : a->rows) {
      if (r.caller_prog) {
        EXTRA_REF(BLOCK(r.caller_prog))++;
      }
      if (r.target_prog) {
        EXTRA_REF(BLOCK(r.target_prog))++;
      }
      if (r.function) {
        EXTRA_REF(BLOCK(r.function))++;
      }
    }
  }
  /* Suspended stacks are marked via mark_stack()'s walk. */
}
#endif

/* Shared miss dispatch for call_other:miss and apply:miss.
 * When origin is non-null (apply:miss), handlers receive it as an extra
 * argument and attachments may filter on the origin spec key. */
HookMissResult dispatch_miss(PointId p, object_t* named_target, const char* fn, int num_arg,
                             const char* reason, const char* origin, svalue_t* out) {
  uint32_t const mask = point_mask(p);
  if (!(g_hook_mask & mask)) {
    return HookMissResult::NoOpinion;
  }
  /* Exact-object index: skip when no attachment can match this target. */
  if (pin_index_rejects(p, named_target)) {
    return HookMissResult::NoOpinion;
  }
  /* Master calls never raise call-type events. */
  if (current_object == master_ob) {
    return HookMissResult::NoOpinion;
  }
  auto list = snapshot_point(p);
  svalue_t* call_args = vm_sp() - num_arg + 1;
  const char* caller_prog =
      (current_object && current_object->prog) ? current_object->prog->filename : nullptr;
  const char* target_prog =
      (named_target && named_target->prog) ? named_target->prog->filename : nullptr;
  bool const use_origin = (origin != nullptr);

  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_BEFORE) {
      continue;
    }
    if (use_origin) {
      if (!attachment_matches_call(a, named_target, fn, origin)) {
        continue;
      }
    } else if (!attachment_matches_miss(a, named_target, fn)) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, caller_prog, target_prog, fn);
      continue;
    }
    /* Move call args aside, push handler args, call, restore call args. */
    std::vector<svalue_t> saved(num_arg);
    for (int i = 0; i < num_arg; i++) {
      saved[i] = call_args[i];
    }
    vm_sp() -= num_arg;
    int n = push_miss_args(a, named_target, fn, saved.data(), num_arg, reason, origin);
    svalue_t tmp{};
    (void)hook_call(a, n, &tmp);
    free_svalue(&tmp, "hook before");
    for (int i = 0; i < num_arg; i++) {
      push_svalue(&saved[i]);
      free_svalue(&saved[i], "hook before restore");
    }
    call_args = vm_sp() - num_arg + 1;
  }

  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_DECIDE) {
      continue;
    }
    if (use_origin) {
      if (!attachment_matches_call(a, named_target, fn, origin)) {
        continue;
      }
    } else if (!attachment_matches_miss(a, named_target, fn)) {
      continue;
    }
    /* Latch before hook_call: detach may delete `a` before we return. */
    int const flags = a->flags;
    std::vector<svalue_t> saved(num_arg);
    for (int i = 0; i < num_arg; i++) {
      saved[i] = call_args[i];
    }
    vm_sp() -= num_arg;
    int n = push_miss_args(a, named_target, fn, saved.data(), num_arg, reason, origin);
    svalue_t result{};
    HookRun run = hook_call(a, n, &result);
    /* apply_low() always pop_n_elems(num_arg) after dispatch — restore. */
    for (int i = 0; i < num_arg; i++) {
      push_svalue(&saved[i]);
      free_svalue(&saved[i], "hook decide restore");
    }
    call_args = vm_sp() - num_arg + 1;
    if (run == HookRun::Skipped && (flags & HOOK_FAIL_CLOSED)) {
      /* miss point: FAIL_CLOSED refused at attach time; treat as no opinion */
      free_svalue(&result, "hook decide skip");
      continue;
    }
    if (run != HookRun::Ran) {
      free_svalue(&result, "hook decide");
      continue;
    }
    /* answer: ({ value }) */
    if (result.type == T_ARRAY && result.u.arr->size == 1) {
      assign_svalue_no_free(out, &result.u.arr->item[0]);
      free_svalue(&result, "hook decide answer");
      return HookMissResult::Answered;
    }
    /* refuse (string or 1) is not defined as changing a miss — no-opinion */
    free_svalue(&result, "hook decide");
  }
  return HookMissResult::NoOpinion;
}

HookMissResult hook_dispatch_call_other_miss(object_t* named_target, const char* fn, int num_arg,
                                            const char* reason, svalue_t* out) {
  return dispatch_miss(PointId::CallOtherMiss, named_target, fn, num_arg, reason, nullptr, out);
}

HookMissResult hook_dispatch_apply_miss(object_t* named_target, const char* fn, int num_arg,
                                       const char* reason, const char* origin, svalue_t* out) {
  return dispatch_miss(PointId::ApplyMiss, named_target, fn, num_arg, reason, origin, out);
}

void hook_dispatch_object_create(object_t* ob) {
  if (!(g_hook_mask & HOOK_MASK_OBJECT_CREATE) || !ob || (ob->flags & O_DESTRUCTED)) {
    return;
  }
  auto list = snapshot_point(PointId::ObjectCreate);
  for (Attachment* a : list) {
    if (a->detached) {
      continue;
    }
    if (!attachment_matches_object(a, ob)) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, nullptr, ob->prog ? ob->prog->filename : nullptr, "create");
      continue;
    }
    push_number(a->id);
    push_object(ob);
    svalue_t tmp{};
    (void)hook_call(a, 2, &tmp);
    free_svalue(&tmp, "hook create");
    if (ob->flags & O_DESTRUCTED) {
      return;
    }
  }
}

void hook_dispatch_object_destruct(object_t* ob, const char* cause) {
  if (!(g_hook_mask & HOOK_MASK_OBJECT_DESTRUCT) || !ob) {
    return;
  }
  auto list = snapshot_point(PointId::ObjectDestruct);
  for (Attachment* a : list) {
    if (a->detached) {
      continue;
    }
    if (!attachment_matches_object(a, ob)) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, nullptr, ob->prog ? ob->prog->filename : nullptr, cause);
      continue;
    }
    push_number(a->id);
    push_object(ob);
    copy_and_push_string(cause ? cause : "efun");
    svalue_t tmp{};
    (void)hook_call(a, 3, &tmp);
    free_svalue(&tmp, "hook destruct");
    /* If the handler destructed `ob`, stop — the cascade owns the rest. */
    if (ob->flags & O_DESTRUCTED) {
      return;
    }
  }
}

HookGateResult hook_dispatch_function_before(object_t* target, const char* fn, const char* origin,
                                             int num_arg, svalue_t* out) {
  if (!(g_hook_mask & HOOK_MASK_LPC_FUNC)) {
    return HookGateResult::NoOpinion;
  }
  if (pin_index_rejects(PointId::Function, target)) {
    return HookGateResult::NoOpinion;
  }
  auto list = snapshot_point(PointId::Function);
  svalue_t* call_args = num_arg > 0 ? vm_sp() - num_arg + 1 : nullptr;
  const char* caller_prog =
      (current_object && current_object->prog) ? current_object->prog->filename : nullptr;
  const char* target_prog = (target && target->prog) ? target->prog->filename : nullptr;

  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_BEFORE) {
      continue;
    }
    if (!attachment_matches_call(a, target, fn, origin)) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, caller_prog, target_prog, fn);
      continue;
    }
    std::vector<svalue_t> saved(num_arg);
    for (int i = 0; i < num_arg; i++) {
      saved[i] = call_args[i];
    }
    vm_sp() -= num_arg;
    int n = push_call_args(a, target, fn, saved.data(), num_arg);
    svalue_t tmp{};
    (void)hook_call(a, n, &tmp);
    free_svalue(&tmp, "hook function before");
    for (int i = 0; i < num_arg; i++) {
      push_svalue(&saved[i]);
      free_svalue(&saved[i], "hook function before restore");
    }
    call_args = num_arg > 0 ? vm_sp() - num_arg + 1 : nullptr;
  }

  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_DECIDE) {
      continue;
    }
    if (!attachment_matches_call(a, target, fn, origin)) {
      continue;
    }
    int const flags = a->flags;
    std::vector<svalue_t> saved(num_arg);
    for (int i = 0; i < num_arg; i++) {
      saved[i] = call_args[i];
    }
    vm_sp() -= num_arg;
    int n = push_call_args(a, target, fn, saved.data(), num_arg);
    svalue_t result{};
    HookRun run = hook_call(a, n, &result);
    for (int i = 0; i < num_arg; i++) {
      push_svalue(&saved[i]);
      free_svalue(&saved[i], "hook function decide restore");
    }
    call_args = num_arg > 0 ? vm_sp() - num_arg + 1 : nullptr;
    HookGateResult gr = decide_result(&result, run, flags, out, true);
    free_svalue(&result, "hook function decide");
    if (gr != HookGateResult::NoOpinion) {
      return gr;
    }
  }
  return HookGateResult::NoOpinion;
}

void hook_dispatch_function_after(object_t* target, const char* fn, const char* origin,
                                  int num_arg) {
  if (!(g_hook_mask & HOOK_MASK_LPC_FUNC) || !vm_sp()) {
    return;
  }
  if (pin_index_rejects(PointId::Function, target)) {
    return;
  }
  (void)num_arg;
  auto list = snapshot_point(PointId::Function);
  /* Return value is on the stack; snapshot it for handlers. */
  svalue_t ret{};
  assign_svalue_no_free(&ret, vm_sp());
  DEFER { free_svalue(&ret, "hook function after ret"); };

  const char* caller_prog =
      (current_object && current_object->prog) ? current_object->prog->filename : nullptr;
  const char* target_prog = (target && target->prog) ? target->prog->filename : nullptr;
  int64_t body_us = 0;
  {
    ptrdiff_t depth = csp - control_stack;
    if (depth >= 0 && depth < CFG_MAX_CALL_DEPTH) {
      body_us = us_since(g_hook_frame_t0[depth]);
    }
  }

  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_AFTER) {
      continue;
    }
    if (!attachment_matches_call(a, target, fn, origin)) {
      continue;
    }
    if (a->is_count) {
      bump_count(a, caller_prog, target_prog, fn, 0);
      continue;
    }
    if (a->is_time) {
      bump_count(a, caller_prog, target_prog, fn, body_us);
      continue;
    }
    /* args not preserved across the body — empty array + result */
    array_t* empty = allocate_empty_array(0);
    push_number(a->id);
    if (command_giver && !(command_giver->flags & O_DESTRUCTED)) {
      push_object(command_giver);
    } else {
      push_number(0);
    }
    if (current_object && !(current_object->flags & O_DESTRUCTED)) {
      push_object(current_object);
    } else {
      push_number(0);
    }
    if (target && !(target->flags & O_DESTRUCTED)) {
      push_object(target);
    } else {
      push_number(0);
    }
    copy_and_push_string(fn ? fn : "");
    push_refed_array(empty);
    push_svalue(&ret);
    svalue_t tmp{};
    (void)hook_call(a, 7, &tmp);
    free_svalue(&tmp, "hook function after");
  }
}

bool hook_dispatch_function_around(object_t* target, const char* fn, const char* origin, int num_arg,
                                   program_t* prog, unsigned short address, function_t* funp) {
  if (!(g_hook_mask & HOOK_MASK_LPC_FUNC) || !prog || !funp) {
    return false;
  }
  if (pin_index_rejects(PointId::Function, target)) {
    return false;
  }
  auto list = snapshot_point(PointId::Function);
  Attachment* around = nullptr;
  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_AROUND) {
      continue;
    }
    if (!attachment_matches_call(a, target, fn, origin)) {
      continue;
    }
    if (a->is_count || a->is_time) {
      continue;
    }
    around = a;
    break; /* highest priority first */
  }
  if (!around) {
    return false;
  }

  /* Park call args, invoke AROUND handler. Result of handler (or proceed) becomes
   * the call result on the stack. */
  std::vector<svalue_t> saved(num_arg);
  for (int i = 0; i < num_arg; i++) {
    saved[i] = *(vm_sp() - num_arg + 1 + i);
  }
  vm_sp() -= num_arg;

  /* AROUND dispatches nest: the handler, or the body it proceeds into, may
   * call another AROUND-hooked function. Keep the enclosing one's state (its
   * parked args, whether it proceeded) and put it back when this one ends. */
  AroundState enclosing = std::move(g_around);
  g_around = AroundState{};
  g_around.active = true;
  g_around.proceeded = false;
  g_around.target = target;
  g_around.fn = fn;
  g_around.origin = origin;
  g_around.num_arg = num_arg;
  g_around.prog = prog;
  g_around.address = address;
  g_around.funp = funp;
  g_around.function_index_offset = function_index_offset;
  g_around.variable_index_offset = variable_index_offset;
  g_around.saved_args = std::move(saved);
  DEFER {
    for (auto& sv : g_around.saved_args) {
      free_svalue(&sv, "hook around saved");
    }
    g_around = std::move(enclosing);
  };

  int n = push_call_args(around, target, fn, g_around.saved_args.data(), num_arg);
  svalue_t result{};
  HookRun run = hook_call(around, n, &result);
  if (run != HookRun::Ran) {
    free_svalue(&result, "hook around fail");
    /* Error / skip / promise: restore args and let the real body run. */
    for (auto& sv : g_around.saved_args) {
      push_svalue(&sv);
    }
    return false;
  }
  if (!g_around.proceeded) {
    /* Handler returned without proceed — its return value is the call result
     * (refuse-with-value). */
  }
  push_svalue(&result);
  free_svalue(&result, "hook around result");
  /* AFTER after AROUND */
  hook_dispatch_function_after(target, fn, origin, 0);
  return true;
}

HookGateResult hook_dispatch_efun_before(const char* efun_name, int num_arg, svalue_t* out,
                                         object_t** filter_ob_out) {
  if (filter_ob_out) {
    *filter_ob_out = nullptr;
  }
  if (!(g_hook_mask & HOOK_MASK_EFUN)) {
    return HookGateResult::NoOpinion;
  }
  auto list = snapshot_point(PointId::Efun);
  svalue_t* call_args = num_arg > 0 ? vm_sp() - num_arg + 1 : nullptr;
  object_t* first_ob = first_object_arg(call_args, num_arg);
  if (filter_ob_out) {
    *filter_ob_out = first_ob;
  }
  const char* caller_prog =
      (current_object && current_object->prog) ? current_object->prog->filename : nullptr;

  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_BEFORE) {
      continue;
    }
    if (!attachment_matches_efun(a, efun_name, first_ob)) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, caller_prog, nullptr, efun_name);
      continue;
    }
    std::vector<svalue_t> saved(num_arg);
    for (int i = 0; i < num_arg; i++) {
      saved[i] = call_args[i];
    }
    vm_sp() -= num_arg;
    int n = push_call_args(a, first_ob, efun_name, saved.data(), num_arg);
    svalue_t tmp{};
    (void)hook_call(a, n, &tmp);
    free_svalue(&tmp, "hook efun before");
    for (int i = 0; i < num_arg; i++) {
      push_svalue(&saved[i]);
      free_svalue(&saved[i], "hook efun before restore");
    }
    call_args = num_arg > 0 ? vm_sp() - num_arg + 1 : nullptr;
    first_ob = first_object_arg(call_args, num_arg);
  }
  if (filter_ob_out) {
    *filter_ob_out = first_ob;
  }

  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_DECIDE) {
      continue;
    }
    if (!attachment_matches_efun(a, efun_name, first_ob)) {
      continue;
    }
    int const flags = a->flags;
    std::vector<svalue_t> saved(num_arg);
    for (int i = 0; i < num_arg; i++) {
      saved[i] = call_args[i];
    }
    vm_sp() -= num_arg;
    int n = push_call_args(a, first_ob, efun_name, saved.data(), num_arg);
    svalue_t result{};
    HookRun run = hook_call(a, n, &result);
    for (int i = 0; i < num_arg; i++) {
      push_svalue(&saved[i]);
      free_svalue(&saved[i], "hook efun decide restore");
    }
    call_args = num_arg > 0 ? vm_sp() - num_arg + 1 : nullptr;
    first_ob = first_object_arg(call_args, num_arg);
    if (filter_ob_out) {
      *filter_ob_out = first_ob;
    }
    /* efun DECIDE: refuse only (no answer) */
    HookGateResult gr = decide_result(&result, run, flags, out, false);
    free_svalue(&result, "hook efun decide");
    if (gr == HookGateResult::Refuse) {
      return HookGateResult::Refuse;
    }
  }
  return HookGateResult::NoOpinion;
}

void hook_dispatch_efun_after(const char* efun_name, object_t* filter_ob, svalue_t* result) {
  if (!(g_hook_mask & HOOK_MASK_EFUN)) {
    return;
  }
  auto list = snapshot_point(PointId::Efun);
  svalue_t ret{};
  if (result) {
    assign_svalue_no_free(&ret, result);
  } else {
    ret = const0;
  }
  DEFER { free_svalue(&ret, "hook efun after ret"); };
  const char* caller_prog =
      (current_object && current_object->prog) ? current_object->prog->filename : nullptr;

  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_AFTER) {
      continue;
    }
    if (!attachment_matches_efun(a, efun_name, filter_ob)) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, caller_prog, nullptr, efun_name);
      continue;
    }
    array_t* empty = allocate_empty_array(0);
    push_number(a->id);
    if (command_giver && !(command_giver->flags & O_DESTRUCTED)) {
      push_object(command_giver);
    } else {
      push_number(0);
    }
    if (current_object && !(current_object->flags & O_DESTRUCTED)) {
      push_object(current_object);
    } else {
      push_number(0);
    }
    /* Same object used for object/target matching (captured before the efun). */
    if (filter_ob && !(filter_ob->flags & O_DESTRUCTED)) {
      push_object(filter_ob);
    } else {
      push_number(0);
    }
    copy_and_push_string(efun_name ? efun_name : "");
    push_refed_array(empty);
    push_svalue(&ret);
    svalue_t tmp{};
    (void)hook_call(a, 7, &tmp);
    free_svalue(&tmp, "hook efun after");
  }
}

bool hook_dispatch_net_accept(const char* addr) {
  if (!(g_hook_mask & HOOK_MASK_NET_ACCEPT)) {
    return true;
  }
  auto list = snapshot_point(PointId::NetAccept);
  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_BEFORE) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, nullptr, nullptr, "accept");
      continue;
    }
    push_number(a->id);
    copy_and_push_string(addr ? addr : "");
    svalue_t tmp{};
    (void)hook_call(a, 2, &tmp);
    free_svalue(&tmp, "hook net accept before");
  }
  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_DECIDE) {
      continue;
    }
    int const flags = a->flags;
    push_number(a->id);
    copy_and_push_string(addr ? addr : "");
    svalue_t result{};
    HookRun run = hook_call(a, 2, &result);
    HookGateResult gr = decide_result(&result, run, flags, nullptr, false);
    free_svalue(&result, "hook net accept decide");
    if (gr == HookGateResult::Refuse) {
      return false;
    }
  }
  return true;
}

/* Push net read/write handler args; returns arity.
 * `data`/`len` are a raw byte span (may contain NULs and need not be
 * NUL-terminated). Copy exactly `len` bytes into an LPC string. */
int push_net_rw_args(Attachment* a, object_t* user_ob, const char* data, int len) {
  push_number(a->id);
  if (user_ob && !(user_ob->flags & O_DESTRUCTED)) {
    push_object(user_ob);
  } else {
    push_number(0);
  }
  if (len < 0) {
    len = 0;
  }
  push_number(len);
  char* str = new_string(static_cast<unsigned int>(len), "hook net rw");
  if (len > 0 && data) {
    memcpy(str, data, static_cast<size_t>(len));
  } else if (len > 0) {
    memset(str, 0, static_cast<size_t>(len));
  }
  str[len] = '\0';
  push_malloced_string(str);
  return 4;
}

bool hook_dispatch_net_read(object_t* user_ob, const char* data, int len) {
  if (!(g_hook_mask & HOOK_MASK_NET_READ)) {
    return true;
  }
  auto list = snapshot_point(PointId::NetRead);
  /* BEFORE observers first, then DECIDE (same order as miss / function / accept). */
  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_BEFORE) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, nullptr, nullptr, "read");
      continue;
    }
    int n = push_net_rw_args(a, user_ob, data, len);
    svalue_t result{};
    (void)hook_call(a, n, &result);
    free_svalue(&result, "hook net read");
  }
  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_DECIDE) {
      continue;
    }
    int n = push_net_rw_args(a, user_ob, data, len);
    svalue_t result{};
    HookRun run = hook_call(a, n, &result);
    HookGateResult gr = decide_result(&result, run, a->flags, nullptr, false);
    free_svalue(&result, "hook net read");
    if (gr == HookGateResult::Refuse) {
      return false;
    }
  }
  return true;
}

bool hook_dispatch_net_write(object_t* user_ob, const char* data, int len) {
  if (!(g_hook_mask & HOOK_MASK_NET_WRITE)) {
    return true;
  }
  auto list = snapshot_point(PointId::NetWrite);
  /* BEFORE, then DECIDE (refuse drops before send). AFTER runs separately
   * via hook_dispatch_net_write_after() once the payload is handed to the
   * transport / telnet layer. */
  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_BEFORE) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, nullptr, nullptr, "write");
      continue;
    }
    int n = push_net_rw_args(a, user_ob, data, len);
    svalue_t result{};
    (void)hook_call(a, n, &result);
    free_svalue(&result, "hook net write");
  }
  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_DECIDE) {
      continue;
    }
    int n = push_net_rw_args(a, user_ob, data, len);
    svalue_t result{};
    HookRun run = hook_call(a, n, &result);
    HookGateResult gr = decide_result(&result, run, a->flags, nullptr, false);
    free_svalue(&result, "hook net write");
    if (gr == HookGateResult::Refuse) {
      return false;
    }
  }
  return true;
}

void hook_dispatch_net_write_after(object_t* user_ob, const char* data, int len) {
  if (!(g_hook_mask & HOOK_MASK_NET_WRITE)) {
    return;
  }
  auto list = snapshot_point(PointId::NetWrite);
  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_AFTER) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, nullptr, nullptr, "write");
      continue;
    }
    int n = push_net_rw_args(a, user_ob, data, len);
    svalue_t result{};
    (void)hook_call(a, n, &result);
    free_svalue(&result, "hook net write after");
  }
}

/* Push net:telnet handler args; returns arity. */
int push_net_telnet_args(Attachment* a, object_t* user_ob, const char* event, const char* option,
                         const char* payload, int payload_len) {
  push_number(a->id);
  if (user_ob && !(user_ob->flags & O_DESTRUCTED)) {
    push_object(user_ob);
  } else {
    push_number(0);
  }
  copy_and_push_string(event ? event : "");
  copy_and_push_string(option ? option : "");
  if (payload_len < 0) {
    payload_len = 0;
  }
  char* str = new_string(static_cast<unsigned int>(payload_len), "hook net telnet");
  if (payload_len > 0 && payload) {
    memcpy(str, payload, static_cast<size_t>(payload_len));
  } else if (payload_len > 0) {
    memset(str, 0, static_cast<size_t>(payload_len));
  }
  str[payload_len] = '\0';
  push_malloced_string(str);
  return 5;
}

bool hook_dispatch_net_telnet(object_t* user_ob, const char* event, const char* option,
                              const char* payload, int payload_len) {
  if (!(g_hook_mask & HOOK_MASK_NET_TELNET)) {
    return true;
  }
  auto list = snapshot_point(PointId::NetTelnet);
  const bool is_sb = event && strcmp(event, "sb") == 0;
  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_BEFORE) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, nullptr, nullptr, event);
      continue;
    }
    int n = push_net_telnet_args(a, user_ob, event, option, payload, payload_len);
    svalue_t tmp{};
    (void)hook_call(a, n, &tmp);
    free_svalue(&tmp, "hook net telnet before");
  }
  /* DECIDE is only meaningful for subnegotiation — negotiation replies
   * have already gone out via libtelnet and cannot be refused. */
  if (is_sb) {
    for (Attachment* a : list) {
      if (a->detached || a->kind != HOOK_DECIDE) {
        continue;
      }
      int n = push_net_telnet_args(a, user_ob, event, option, payload, payload_len);
      svalue_t result{};
      HookRun run = hook_call(a, n, &result);
      HookGateResult gr = decide_result(&result, run, a->flags, nullptr, false);
      free_svalue(&result, "hook net telnet decide");
      if (gr == HookGateResult::Refuse) {
        return false;
      }
    }
  }
  return true;
}

void hook_dispatch_net_tls(object_t* user_ob, int port, const char* version, const char* cipher,
                           const char* outcome) {
  if (!(g_hook_mask & HOOK_MASK_NET_TLS)) {
    return;
  }
  auto list = snapshot_point(PointId::NetTls);
  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_AFTER) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, nullptr, nullptr, outcome);
      continue;
    }
    push_number(a->id);
    if (user_ob && !(user_ob->flags & O_DESTRUCTED)) {
      push_object(user_ob);
    } else {
      push_number(0);
    }
    push_number(port);
    copy_and_push_string(version ? version : "");
    copy_and_push_string(cipher ? cipher : "");
    copy_and_push_string(outcome ? outcome : "");
    svalue_t tmp{};
    (void)hook_call(a, 6, &tmp);
    free_svalue(&tmp, "hook net tls");
  }
}

void hook_dispatch_net_resolve(const char* address, const char* name, object_t* user_ob) {
  if (!(g_hook_mask & HOOK_MASK_NET_RESOLVE)) {
    return;
  }
  auto list = snapshot_point(PointId::NetResolve);
  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_AFTER) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, nullptr, nullptr, "resolve");
      continue;
    }
    push_number(a->id);
    copy_and_push_string(address ? address : "");
    copy_and_push_string(name ? name : "");
    if (user_ob && !(user_ob->flags & O_DESTRUCTED)) {
      push_object(user_ob);
    } else {
      push_number(0);
    }
    svalue_t tmp{};
    (void)hook_call(a, 4, &tmp);
    free_svalue(&tmp, "hook net resolve");
  }
}

bool hook_dispatch_net_ws_upgrade(const char* addr) {
  if (!(g_hook_mask & HOOK_MASK_NET_WS_UPGRADE)) {
    return true;
  }
  auto list = snapshot_point(PointId::NetWsUpgrade);
  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_BEFORE) {
      continue;
    }
    if (a->is_count || a->is_time) {
      bump_count(a, nullptr, nullptr, "ws_upgrade");
      continue;
    }
    push_number(a->id);
    copy_and_push_string(addr ? addr : "");
    svalue_t tmp{};
    (void)hook_call(a, 2, &tmp);
    free_svalue(&tmp, "hook net ws before");
  }
  for (Attachment* a : list) {
    if (a->detached || a->kind != HOOK_DECIDE) {
      continue;
    }
    push_number(a->id);
    copy_and_push_string(addr ? addr : "");
    svalue_t result{};
    HookRun run = hook_call(a, 2, &result);
    HookGateResult gr = decide_result(&result, run, a->flags, nullptr, false);
    free_svalue(&result, "hook net ws decide");
    if (gr == HookGateResult::Refuse) {
      return false;
    }
  }
  return true;
}

bool hooks_around_active() { return g_around.active; }

void hooks_mark_frame_for_after(object_t* filter_target) {
  /* Caller already tested HOOK_MASK_LPC_FUNC_AFTER. */
  csp->framekind |= FRAME_HOOKED;
  csp->hook_target = filter_target;
  if (g_hook_mask & HOOK_MASK_TIME) {
    ptrdiff_t depth = csp - control_stack;
    if (depth >= 0 && depth < CFG_MAX_CALL_DEPTH) {
      g_hook_frame_t0[depth] = std::chrono::steady_clock::now();
    }
  }
}

void f_hook_proceed(int num_arg) {
  int const num = num_arg;
  if (!g_around.active) {
    error("hook_proceed: no active HOOK_AROUND handler.\n");
  }
  if (g_around.proceeded) {
    error("hook_proceed: already called.\n");
  }
  if (!g_around.prog || !g_around.funp || !g_around.target) {
    error("hook_proceed: around context incomplete.\n");
  }
  if (g_around.target->flags & O_DESTRUCTED) {
    error("hook_proceed: target object is destructed.\n");
  }

  /* Optional replacement args: hook_proceed(mixed *args) */
  std::vector<svalue_t> use_args;
  if (num == 1) {
    if (vm_sp()->type != T_ARRAY) {
      error("Bad argument 1 to hook_proceed()\n");
    }
    array_t* arr = vm_sp()->u.arr;
    use_args.resize(arr->size);
    for (int i = 0; i < arr->size; i++) {
      assign_svalue_no_free(&use_args[i], &arr->item[i]);
    }
    pop_stack();
  } else if (num == 0) {
    use_args.resize(g_around.saved_args.size());
    for (size_t i = 0; i < g_around.saved_args.size(); i++) {
      assign_svalue_no_free(&use_args[i], &g_around.saved_args[i]);
    }
  } else {
    error("Wrong number of arguments to hook_proceed()\n");
  }
  g_around.proceeded = true;

  object_t* target = g_around.target;
  program_t* prog = g_around.prog;
  function_t* funp = g_around.funp;
  unsigned short address = g_around.address;
  int narg = static_cast<int>(use_args.size());
  int fio = g_around.function_index_offset;
  int vio = g_around.variable_index_offset;

  for (auto& sv : use_args) {
    push_svalue(&sv);
    free_svalue(&sv, "hook_proceed arg");
  }

  /* Run the real body; eval_instruction marks this frame FRAME_EXTERNAL so
   * F_RETURN unwinds back here. */
  push_control_stack(FRAME_FUNCTION | FRAME_OB_CHANGE);
  current_prog = prog;
  function_index_offset = fio;
  variable_index_offset = vio;
  caller_type = ORIGIN_DRIVER;
  csp->num_local_variables = narg;
  csp->fr.table_index = funp - prog->function_table;
  previous_ob = current_object;
  current_object = target;
  if (funp->num_arg || funp->num_local) {
    setup_variables(narg, funp->num_local, funp->num_arg);
  } else {
    setup_variables(narg, 0, narg);
  }
  call_program(prog, address);
  /* Result left on the stack as hook_proceed's return value. */
  (void)address;
}

void hooks_owner_destructed(object_t* ob) {
  if (g_attachments.empty()) {
    return;
  }
  std::vector<Attachment*> doomed;
  for (auto& kv : g_attachments) {
    Attachment* a = kv.second;
    if (a->owner == ob || (a->fp && a->fp->hdr.owner == ob) || a->filter_object == ob ||
        a->filter_caller_object == ob) {
      doomed.push_back(a);
    }
  }
  for (Attachment* a : doomed) {
    detach_attachment(a);
  }
}

void hooks_revalidate_all() {
  /* valid_hook() is LPC and may detach any attachment, erasing it from
   * g_attachments: walk a copy, inside a dispatch scope. */
  HookDispatchScope const scope;
  std::vector<Attachment*> all;
  all.reserve(g_attachments.size());
  for (auto& kv : g_attachments) {
    all.push_back(kv.second);
  }
  std::vector<Attachment*> doomed;
  for (Attachment* a : all) {
    if (a->detached) {
      continue;
    }
    /* Rebuild a minimal spec and re-ask. For simplicity, re-attach check:
     * if valid_hook is absent or returns < needed tier, detach. */
    mapping_t* spec = allocate_mapping(4);
    if (a->filter_object && !(a->filter_object->flags & O_DESTRUCTED)) {
      add_mapping_object(spec, "object", a->filter_object);
    }
    if (a->filter_caller_object && !(a->filter_caller_object->flags & O_DESTRUCTED)) {
      add_mapping_object(spec, "caller_object", a->filter_caller_object);
    }
    if (a->filter_function) {
      add_mapping_shared_string(spec, "function", const_cast<char*>(a->filter_function));
    }
    if (a->filter_target) {
      add_mapping_shared_string(spec, "target", const_cast<char*>(a->filter_target));
    }
    if (a->filter_caller) {
      add_mapping_shared_string(spec, "caller", const_cast<char*>(a->filter_caller));
    }
    if (a->filter_caller_program) {
      add_mapping_shared_string(spec, "caller_program",
                               const_cast<char*>(a->filter_caller_program));
    }
    if (a->filter_origin) {
      add_mapping_shared_string(spec, "origin", const_cast<char*>(a->filter_origin));
    }
    if (a->exclude_caller_program) {
      add_mapping_shared_string(spec, "exclude_caller_program",
                               const_cast<char*>(a->exclude_caller_program));
    }
    bool has_scope = a->filter_object || a->filter_caller_object || a->filter_target ||
                     a->filter_caller || a->filter_caller_program || a->filter_function ||
                     a->filter_origin || a->exclude_caller_program ||
                     (a->point == PointId::Efun && a->efun_name && strcmp(a->efun_name, "*") != 0);
    int needed =
        compute_tier(a->point, a->kind, a->flags, a->owner, a->fp, a->filter_object,
                     a->filter_caller_object, has_scope);
    svalue_t action{};
    if (a->fp) {
      action.type = T_FUNCTION;
      action.u.fp = a->fp;
    } else if (a->is_count) {
      action.type = T_STRING;
      action.subtype = STRING_CONSTANT;
      action.u.string = "count";
    } else {
      action.type = T_STRING;
      action.subtype = STRING_CONSTANT;
      action.u.string = "time";
    }
    int granted = 0;
    if (master_ob) {
      push_object(a->owner);
      copy_and_push_string(a->point_string ? a->point_string : point_name(a->point));
      push_svalue(&action);
      spec->ref++;
      push_mapping(spec);
      push_number(a->flags);
      push_number(needed);
      svalue_t* ret = safe_apply_master_ob(APPLY_VALID_HOOK, 6);
      if (ret && ret != (svalue_t*)-1 && ret->type == T_NUMBER) {
        granted = static_cast<int>(ret->u.number);
      }
    }
    free_mapping(spec);
    if (granted < needed) {
      doomed.push_back(a);
    }
  }
  for (Attachment* a : doomed) {
    detach_attachment(a);
  }
}

int64_t hook_efun_attach(const char* point, svalue_t* action, mapping_t* spec, int flags) {
  if (!CONFIG_INT(__RC_HOOKS_ENABLED__)) {
    error("hook_attach: hooks are disabled (hooks enabled : 0).\n");
  }
  const char* parsed_efun = nullptr;
  PointId p = parse_point(point, &parsed_efun);
  if (p == PointId::Count) {
    error("hook_attach: unknown join point '%s'.\n", point ? point : "");
  }
  /* RAII-free the parsed efun name if we error before storing it. */
  DEFER {
    if (parsed_efun) {
      free_string(parsed_efun);
      parsed_efun = nullptr;
    }
  };
  int raw_kind = kind_from_flags(flags);
  if (raw_kind < 0) {
    error("hook_attach: flags must set exactly one of HOOK_BEFORE/DECIDE/AFTER/AROUND.\n");
  }
  if (!kind_allowed(p, raw_kind)) {
    error("hook_attach: kind not allowed for point '%s'.\n", point);
  }
  int kind = normalize_kind(p, flags);
  if ((flags & HOOK_FAIL_CLOSED) &&
      (p == PointId::CallOtherMiss || p == PointId::ApplyMiss)) {
    error("hook_attach: HOOK_FAIL_CLOSED is not allowed on %s.\n", point_name(p));
  }
  if (kind == HOOK_AROUND && p != PointId::Function) {
    error("hook_attach: HOOK_AROUND is only allowed on function.\n");
  }

  int max_att = CONFIG_INT(__RC_HOOK_MAX_ATTACHMENTS__);
  if (max_att <= 0) {
    max_att = 10000;
  }
  if (static_cast<int>(g_attachments.size()) >= max_att) {
    error("hook_attach: hook max attachments reached.\n");
  }

  object_t* who = current_object;
  if (!who || (who->flags & O_DESTRUCTED)) {
    error("hook_attach: no current object.\n");
  }

  bool is_count = false, is_time = false;
  funptr_t* fp = nullptr;
  if (action->type == T_FUNCTION) {
    fp = action->u.fp;
  } else if (action->type == T_STRING) {
    if (strcmp(action->u.string, "count") == 0) {
      is_count = true;
    } else if (strcmp(action->u.string, "time") == 0) {
      is_time = true;
      if (p != PointId::Function || kind != HOOK_AFTER) {
        error("hook_attach: \"time\" is only valid with function + HOOK_AFTER.\n");
      }
    } else {
      error("hook_attach: action must be a function, \"count\", or \"time\".\n");
    }
  } else {
    error("hook_attach: action must be a function, \"count\", or \"time\".\n");
  }
  if ((is_count || is_time) && kind == HOOK_AROUND) {
    error("hook_attach: \"count\"/\"time\" cannot be HOOK_AROUND.\n");
  }
  if ((is_count || is_time) && kind == HOOK_DECIDE) {
    error("hook_attach: \"count\"/\"time\" cannot be HOOK_DECIDE.\n");
  }
  if (flags & HOOK_DEFERRED) {
    error("hook_attach: HOOK_DEFERRED is reserved and not yet implemented.\n");
  }
  if (flags & HOOK_OVERRIDE_NOMASK) {
    error("hook_attach: HOOK_OVERRIDE_NOMASK is reserved and not yet implemented.\n");
  }

  object_t* f_obj = nullptr;
  const char* f_obj_str = nullptr;
  read_object_or_string_key(spec, "object", &f_obj, &f_obj_str);
  object_t* f_cobj = nullptr;
  const char* f_cobj_str = nullptr;
  read_object_or_string_key(spec, "caller_object", &f_cobj, &f_cobj_str);
  const char* f_fn = read_string_key(spec, "function");
  const char* f_target = read_string_key(spec, "target");
  const char* f_caller = read_string_key(spec, "caller");
  const char* f_cprog = read_string_key(spec, "caller_program");
  const char* f_origin = read_string_key(spec, "origin");
  const char* f_excl_cprog = read_string_key(spec, "exclude_caller_program");
  const char* label = read_string_key(spec, "label");
  int priority = read_number_key(spec, "priority", 0);
  int expires = read_number_key(spec, "expires", 0);

  /* String form of object/caller_object is a name pattern (same as target/caller). */
  if (f_obj_str) {
    if (f_target) {
      error("hook_attach: use object (string) or target, not both.\n");
    }
    f_target = f_obj_str;
  }
  if (f_cobj_str) {
    if (f_caller) {
      error("hook_attach: use caller_object (string) or caller, not both.\n");
    }
    f_caller = f_cobj_str;
  }

  validate_name_pattern(f_target, "object/target");
  validate_name_pattern(f_caller, "caller_object/caller");
  validate_name_pattern(f_fn, "function");
  validate_name_pattern(f_cprog, "caller_program");
  validate_name_pattern(f_excl_cprog, "exclude_caller_program");
  if (parsed_efun) {
    validate_name_pattern(parsed_efun, "efun name");
  }

  /* call_other:miss / apply:miss / function run on hot paths. Require an
   * object pin or target path (prefix / "*" OK); function/origin filters
   * alone are not enough. */
  if ((p == PointId::CallOtherMiss || p == PointId::ApplyMiss || p == PointId::Function) &&
      !(f_obj || f_target)) {
    error("hook_attach: %s requires object or target.\n", point_name(p));
  }

  bool has_scope = f_obj || f_cobj || f_target || f_caller || f_cprog || f_fn || f_origin ||
                   f_excl_cprog ||
                   (p == PointId::Efun && parsed_efun && strcmp(parsed_efun, "*") != 0);
  int needed = compute_tier(p, kind, flags, who, fp, f_obj, f_cobj, has_scope);

  /* Fresh mapping copy for the master. */
  mapping_t* for_master = allocate_mapping(8);
  if (f_obj) {
    add_mapping_object(for_master, "object", f_obj);
  }
  if (f_cobj) {
    add_mapping_object(for_master, "caller_object", f_cobj);
  }
  if (f_fn) {
    add_mapping_string(for_master, "function", f_fn);
  }
  if (f_target) {
    const char* canon = canonicalize_name(f_target);
    add_mapping_shared_string(for_master, "target", const_cast<char*>(canon));
    free_string(canon);
  }
  if (f_caller) {
    const char* canon = canonicalize_name(f_caller);
    add_mapping_shared_string(for_master, "caller", const_cast<char*>(canon));
    free_string(canon);
  }
  if (f_cprog) {
    add_mapping_string(for_master, "caller_program", f_cprog);
  }
  if (f_origin) {
    add_mapping_string(for_master, "origin", f_origin);
  }
  if (f_excl_cprog) {
    add_mapping_string(for_master, "exclude_caller_program", f_excl_cprog);
  }
  if (label) {
    add_mapping_string(for_master, "label", label);
  }
  add_mapping_pair(for_master, "priority", priority);
  if (expires) {
    add_mapping_pair(for_master, "expires", expires);
  }

  int granted = 0;
  if (master_ob) {
    push_object(who);
    copy_and_push_string(point);
    push_svalue(action);
    push_refed_mapping(for_master);
    push_number(flags);
    push_number(needed);
    svalue_t* ret = safe_apply_master_ob(APPLY_VALID_HOOK, 6);
    if (ret && ret != (svalue_t*)-1 && ret->type == T_NUMBER) {
      granted = static_cast<int>(ret->u.number);
    }
  } else {
    free_mapping(for_master);
  }
  if (granted < needed) {
    error("hook_attach: permission denied by valid_hook() (needed tier %d, granted %d).\n", needed,
          granted);
  }

  auto* a = new Attachment();
  a->id = g_next_id++;
  a->point = p;
  a->point_string = make_shared_string(point);
  a->flags = flags;
  a->kind = kind;
  a->priority = priority;
  /* Spec "expires" is a TTL in wall-clock seconds from attach; stored absolute. */
  a->expires = expires > 0 ? get_current_time() + expires : 0;
  a->owner = who;
  add_ref(who, "hook owner");
  a->is_count = is_count;
  a->is_time = is_time;
  if (fp) {
    a->fp = fp;
    fp->hdr.ref++;
  }
  if (f_obj) {
    a->filter_object = f_obj;
    add_ref(f_obj, "hook filter_object");
  }
  if (f_cobj) {
    a->filter_caller_object = f_cobj;
    add_ref(f_cobj, "hook filter_caller_object");
  }
  if (f_fn) {
    a->filter_function = make_shared_string(f_fn);
  }
  if (f_target) {
    a->filter_target = canonicalize_name(f_target);
  }
  if (f_caller) {
    a->filter_caller = canonicalize_name(f_caller);
  }
  if (f_cprog) {
    a->filter_caller_program = make_shared_string(f_cprog);
  }
  if (f_origin) {
    a->filter_origin = make_shared_string(f_origin);
  }
  if (f_excl_cprog) {
    a->exclude_caller_program = make_shared_string(f_excl_cprog);
  }
  if (parsed_efun) {
    const char* canon = canonical_efun_name(parsed_efun);
    if (strcmp(canon, parsed_efun) != 0) {
      free_string(parsed_efun);
      a->efun_name = make_shared_string(canon);
    } else {
      a->efun_name = parsed_efun;
    }
    parsed_efun = nullptr; /* ownership transferred; DEFER no-ops */
  }
  if (label) {
    a->label = make_shared_string(label);
  }

  g_attachments[a->id] = a;
  g_by_point[static_cast<int>(p)].push_back(a);
  sort_point(p);
  recompute_mask();
  return a->id;
}

void hook_efun_detach(int64_t id) {
  auto it = g_attachments.find(id);
  if (it == g_attachments.end()) {
    return;
  }
  Attachment* a = it->second;
  if (current_object != a->owner && current_object != master_ob) {
    error("hook_detach: only the owner or the master may detach.\n");
  }
  detach_attachment(a);
}

mapping_t* hook_efun_query(int64_t id) {
  auto it = g_attachments.find(id);
  if (it == g_attachments.end()) {
    return allocate_mapping(0);
  }
  Attachment* a = it->second;
  if (current_object != a->owner && current_object != master_ob) {
    error("hook_query: only the owner or the master may query.\n");
  }
  return attachment_to_mapping(a, true);
}

array_t* hook_efun_list(object_t* ob) {
  std::vector<mapping_t*> maps;
  if (!ob) {
    for (auto& kv : g_attachments) {
      Attachment* a = kv.second;
      if (current_object == master_ob || a->owner == current_object) {
        maps.push_back(attachment_to_mapping(a, false));
      }
    }
  } else {
    for (auto& kv : g_attachments) {
      Attachment* a = kv.second;
      bool applies = (a->filter_object && object_filter_matches(a->filter_object, ob)) ||
                     (a->filter_target && object_name_matches(a->filter_target, ob->obname));
      if (!applies) {
        continue;
      }
      if (a->owner == current_object || current_object == master_ob) {
        maps.push_back(attachment_to_mapping(a, false));
      } else {
        mapping_t* m = allocate_mapping(1);
        if (a->label) {
          add_mapping_shared_string(m, "label", const_cast<char*>(a->label));
        }
        maps.push_back(m);
      }
    }
  }
  array_t* arr = allocate_empty_array(static_cast<int>(maps.size()));
  for (size_t i = 0; i < maps.size(); i++) {
    arr->item[i].type = T_MAPPING;
    arr->item[i].u.map = maps[i];
  }
  return arr;
}

void hooks_on_set_eval() {
  if (!g_hook_exempt) {
    g_hook_eval_credit = 0;
    g_hook_allow_used = 0;
    refresh_eval_active();
  }
}

int64_t hooks_eval_credit() { return g_hook_eval_credit; }
