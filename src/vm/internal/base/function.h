#ifndef FUNCTION_H
#define FUNCTION_H

/* It is usually better to include "lpc_incl.h" instead of including this
   directly */

/* FP_SIMUL / FP_EFUN */
typedef struct {
  short index;
} simul_ptr_t;
typedef simul_ptr_t efun_ptr_t;

/* FP_LOCAL */
typedef struct {
  short index;
  /* The slot the reference named before FUNC_ALIAS resolution (an override's
     inherited slot still names the inherited function). It is what identifies
     the function by name when recompile_object() re-resolves the pointer. */
  short ref_index;
  /* The owner's program when the pointer was made: `index` is relative
     to ITS function table, and it is the program whose func_ref this
     pointer holds. The owner's live prog can move on (recompile_object),
     so creation/destruction accounting must use this one. */
  struct program_t* prog;
} local_ptr_t;

/* FP_FUNCTIONAL */
struct functional_t {
  /* these two must come first */
  unsigned char num_arg;
  unsigned char num_local;
  short fio;
  struct program_t* prog;
  int offset;
  short vio;
  // char lpccode[80];
};

/* common header */
struct funptr_hdr_t {
  uint32_t ref;
  short type; /* FP_* is used */
#ifdef DEBUGMALLOC_EXTENSIONS
  int extra_ref;
#endif
  /* owner->prog_generation at creation/bind time. FP_LOCAL indices and
     FP_FUNCTIONAL variable offsets are relative to the owner's program
     LAYOUT at that moment; if recompile_object() has swapped the program
     since, calling through them would run the wrong function or corrupt
     variables -- the call path compares generations and errors
     cleanly instead. */
  uint32_t owner_gen;
  struct object_t* owner;
  struct array_t* args;
};

struct funptr_t {
  funptr_hdr_t hdr;
  union {
    efun_ptr_t efun;
    local_ptr_t local;
    simul_ptr_t simul;
    functional_t functional;
  } f;
};

union string_or_func {
  funptr_t* f;
  const char* s;
};

void dealloc_funp(funptr_t*);
/* Drop fp from the named-function intern table (function.cc) if it is the
 * entry. Must run before anything clears fp->hdr.owner. */
void unintern_funp(funptr_t*);
/* Re-resolve an FP_LOCAL pointer whose owner was recompiled against the
 * owner's current program, by defining program and function name. Returns
 * false (pointer left stale) if that function no longer exists. */
bool refresh_local_funp(funptr_t*);
/* Refresh every interned local-function pointer owned by ob. Called by
 * recompile_object() right after the program swap, before new code runs. */
void refresh_named_funps(object_t*);
void push_refed_funp(funptr_t*);
void push_funp(funptr_t*);
void free_funp(funptr_t*);
int merge_arg_lists(int, struct array_t*, int);
funptr_t* make_efun_funp(int, struct svalue_t*);
funptr_t* make_lfun_funp(int, struct svalue_t*);
funptr_t* make_simul_funp(int, struct svalue_t*);
funptr_t* make_functional_funp(short, short, short, struct svalue_t*, int);

#endif
