/* interpret.h */

/* It is usually better to include "lpc_incl.h" instead of including this
   directly */

#ifndef INTERPRET_H
#define INTERPRET_H

#include "vm/internal/base/program.h"
#include "vm/internal/base/svalue.h"

/*
 * Control stack element.
 * 'prog' is usually same as 'ob->prog' (current_object), except when
 * when the current function is defined by inheritance.
 * The pointer, csp, will point to the values that will be used at return.
 */
#define FRAME_FUNCTION 0
#define FRAME_FUNP 1
#define FRAME_CATCH 2
#define FRAME_FAKE 3
#define FRAME_MASK 3

#define FRAME_OB_CHANGE 4
#define FRAME_EXTERNAL 8

#define FRAME_RETURNED_FROM_CATCH 16
struct defer_list {
  struct defer_list *next;
  svalue_t func;
  svalue_t tp;
};
struct control_stack_t {
#ifdef PROFILE_FUNCTIONS
  unsigned long entry_secs, entry_usecs;
#endif
  union {
    long table_index;
    funptr_t *funp;
  } fr;
  object_t *ob;      /* Current object */
  object_t *prev_ob; /* Save previous object */
  program_t *prog;   /* Current program */
  char *pc;          /* TODO: change this to unsigned char* */

  svalue_t *fp;
  struct defer_list *defers;
  int num_local_variables;   /* Local + arguments */
  int function_index_offset; /* Used when executing functions in inherited
                              * programs */
  int variable_index_offset; /* Same */
  short caller_type;         /* was this a locally called function? */
  short framekind;

  std::shared_ptr<std::string> trace_id;
};

struct function_to_call_t {
  object_t *ob;
  union {
    funptr_t *fp;
    const char *str;
  } f;
  int narg;
  svalue_t *args;
};

struct error_context_t {
  struct control_stack_t *save_csp;
  struct svalue_t *save_sp;
  struct object_t **save_cgsp;
  struct error_context_t *save_context;
};

struct function_lookup_info_t {
  function_t *func;
  int index;
};

#define IS_ZERO(x) (!(x) || (((x)->type == T_NUMBER) && ((x)->u.number == 0)))
#define IS_UNDEFINED(x) \
  (!(x) || (((x)->type == T_NUMBER) && ((x)->subtype == T_UNDEFINED) && ((x)->u.number == 0)))

#define CHECK_TYPES(val, t, arg, inst) \
  if (!((val)->type & (t))) bad_argument(val, t, arg, inst);

/* macro calls */
#define call_program(prog, offset) eval_instruction(prog->program + offset)

#define CHECK_STACK_OVERFLOW(x) \
  if (sp + (x) >= end_of_stack) SAFE(too_deep_error = 1; error("stack overflow");)
#define STACK_INC SAFE(CHECK_STACK_OVERFLOW(1); sp++;)

#define push_svalue(x) SAFE(STACK_INC; assign_svalue_no_free(sp, x);)
#define put_number(x) SAFE(sp->type = T_NUMBER; sp->subtype = 0; sp->u.number = (x);)
#define put_buffer(x) SAFE(sp->type = T_BUFFER; sp->u.buf = (x);)
#define put_undested_object(x) SAFE(sp->type = T_OBJECT; sp->u.ob = (x);)
#define put_object(x) \
  SAFE(if (!(x) || (x)->flags & O_DESTRUCTED) *sp = const0u; else put_undested_object(x);)
#define put_unrefed_undested_object(x, y) \
  SAFE(sp->type = T_OBJECT; sp->u.ob = (x); add_ref((x), y);)
#define put_unrefed_object(x, y)                             \
  SAFE(if (!(x) || (x)->flags & O_DESTRUCTED) *sp = const0u; \
       else put_unrefed_undested_object(x, y);)
/* see comments on push_constant_string */
#define put_constant_string(x) \
  SAFE(sp->type = T_STRING; sp->subtype = STRING_SHARED; sp->u.string = make_shared_string(x);)
#define put_malloced_string(x) \
  SAFE(sp->type = T_STRING; sp->subtype = STRING_MALLOC; sp->u.string = (x);)
#define put_array(x) SAFE(sp->type = T_ARRAY; sp->u.arr = (x);)
#define put_shared_string(x) \
  SAFE(sp->type = T_STRING; sp->subtype = STRING_SHARED; sp->u.string = (x);)

extern program_t *current_prog;
extern short caller_type;
extern char *pc;
extern svalue_t *sp;
extern svalue_t *fp;
extern svalue_t *const end_of_stack;
extern svalue_t catch_value;
extern control_stack_t *const control_stack;
extern control_stack_t *csp;
extern int too_deep_error;
extern int max_eval_error;
extern int function_index_offset;
extern int variable_index_offset;
extern int simul_efun_is_loading;
extern program_t fake_prog;
extern svalue_t global_lvalue_byte;
extern int num_varargs;
extern int st_num_arg;

extern ref_t *global_ref_list;
extern int lv_owner_type;
extern refed_t *lv_owner;

void kill_ref(ref_t *);
ref_t *make_ref(void);

void call_direct(object_t *, int, int, int);
void eval_instruction(char *p);

function_t *setup_inherited_frame(int);
program_t *find_function_by_name(object_t *, const char *, int *, int *);
char *function_name(program_t *, int);
void remove_object_from_stack(object_t *);
void setup_fake_frame(funptr_t *);
void remove_fake_frame(void);
void push_indexed_lvalue(int);
void setup_variables(int, int, int);

void process_efun_callback(int, function_to_call_t *, int);
svalue_t *call_efun_callback(function_to_call_t *, int);
svalue_t *safe_call_efun_callback(function_to_call_t *, int);
const char *type_name(int c);
[[noreturn]] void bad_arg(int, int);
[[noreturn]] void bad_argument(svalue_t *, int, int, int);
void check_for_destr(array_t *);
int is_static(const char *, object_t *);
svalue_t *call_function_pointer(funptr_t *, int);
svalue_t *safe_call_function_pointer(funptr_t *, int);
void call___INIT(object_t *);
array_t *call_all_other(array_t *, const char *, int);
const char *function_exists(const char *, object_t *, int);
void call_function(program_t *, int);
void mark_apply_low_cache(void);
void translate_absolute_line(int, unsigned short *, int *, int *);
char *add_slash(const char *const);
int strpref(const char *, const char *);
void do_trace(const char *, const char *, const char *);
void opcdump(const char *);
int inter_sscanf(svalue_t *, svalue_t *, svalue_t *, int);
char *get_line_number_if_any(void);
char *get_line_number(char *, const program_t *);
void get_line_number_info(const char **, int *);
void reset_machine(int);
void unlink_string_svalue(svalue_t *);
void copy_lvalue_range(svalue_t *);
void assign_lvalue_range(svalue_t *);
void debug_perror(const char *, const char *);

#ifndef NO_SHADOWS
int validate_shadowing(object_t *);
#endif

void try_reset(object_t *);

void pop_context(error_context_t *);
void restore_context(error_context_t *);
void save_context(error_context_t *);

void pop_control_stack(void);
function_t *setup_new_frame(int);
void push_control_stack(int);

void break_point(void);

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_svalue(svalue_t *);
void mark_stack(void);
#endif

// TODO: move these to correct places
void setup_varargs_variables(int, int, int);

inline const char *access_to_name(int mode) {
  switch (mode) {
    case DECL_HIDDEN:
      return "hidden";
    case DECL_PRIVATE:
      return "private";
    case DECL_PROTECTED:
      return "protected";
    case DECL_PUBLIC:
      return "public";
#ifndef SENSIBLE_MODIFIERS
    case DECL_VISIBLE:
      return "visible";
#endif
    default:
      return "unknown";
  }
}

void get_explicit_line_number_info(char *, const program_t *, const char **, int *);
int last_instructions();

#endif /* _INTERPRET_H */
