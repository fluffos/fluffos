/* interpret.h */

/* It is usually better to include "lpc_incl.h" instead of including this
   directly */

#ifndef INTERPRET_H
#define INTERPRET_H

#define PUSH_STRING    (0 << 6)
#define PUSH_NUMBER    (1 << 6)
#define PUSH_GLOBAL    (2 << 6)
#define PUSH_LOCAL     (3 << 6)

#define PUSH_WHAT      (3 << 6)
#define PUSH_MASK      (0xff ^ (PUSH_WHAT))

#define SWITCH_CASE_SIZE (2 + sizeof(char *))

/* Trace defines */
#ifdef TRACE
#  define TRACE_CALL 1
#  define TRACE_CALL_OTHER 2
#  define TRACE_RETURN 4
#  define TRACE_ARGS 8
#  define TRACE_EXEC 16
#  define TRACE_HEART_BEAT 32
#  define TRACE_APPLY 64
#  define TRACE_OBJNAME 128
#  define TRACETST(b) (command_giver->interactive->trace_level & (b))
#  define TRACEP(b) \
    (command_giver && command_giver->interactive && TRACETST(b) && \
     (command_giver->interactive->trace_prefix == 0 || \
      (current_object && strpref(command_giver->interactive->trace_prefix, \
              current_object->obname))) )
#  define TRACEHB (current_heart_beat == 0 || (command_giver->interactive->trace_level & TRACE_HEART_BEAT))
#endif

#define EXTRACT_UCHAR(p) (*(unsigned char *)(p))

/* GCC's optimiser makes a reasonable job of optimising the READ_USHORT macro.
 * -- Qwer
 */

#define READ_UCHAR(p) (*(unsigned char *) (p++))
#define READ_USHORT(p) (p++, p++, *((unsigned short *) (p - 2)))

/*
 * Control stack element.
 * 'prog' is usually same as 'ob->prog' (current_object), except when
 * when the current function is defined by inheritance.
 * The pointer, csp, will point to the values that will be used at return.
 */
#define FRAME_FUNCTION     0
#define FRAME_FUNP         1
#define FRAME_CATCH        2
#define FRAME_FAKE         3
#define FRAME_MASK         3

#define FRAME_OB_CHANGE    4
#define FRAME_EXTERNAL     8

#define FRAME_RETURNED_FROM_CATCH   16
struct defer_list{
	struct defer_list *next;
	svalue_t func;
	svalue_t tp;
};
typedef struct control_stack_s {
#ifdef PROFILE_FUNCTIONS
    unsigned long entry_secs, entry_usecs;
#endif
    union {
        long table_index;
        funptr_t *funp;
    } fr;
    object_t *ob;               /* Current object */
    object_t *prev_ob;  /* Save previous object */
    program_t *prog;    /* Current program */
    char *pc;
    svalue_t *fp;
    struct defer_list *defers;
    int num_local_variables;    /* Local + arguments */
    int function_index_offset;  /* Used when executing functions in inherited
                                 * programs */
    int variable_index_offset;  /* Same */
    short caller_type;          /* was this a locally called function? */
    short framekind;
} control_stack_t;

typedef struct {
    object_t *ob;
    union {
        funptr_t *fp;
        const char *str;
    } f;
    int narg;
    svalue_t *args;
} function_to_call_t;

typedef struct error_context_s {
    jmp_buf context;
    control_stack_t *save_csp;
    svalue_t *save_sp;
    object_t **save_cgsp;
    struct error_context_s *save_context;
} error_context_t;

typedef struct {
    function_t *func;
    int index;
} function_lookup_info_t;

#define IS_ZERO(x) (!(x) || (((x)->type == T_NUMBER) && ((x)->u.number == 0)))
#define IS_UNDEFINED(x) (!(x) || (((x)->type == T_NUMBER) && \
        ((x)->subtype == T_UNDEFINED) && ((x)->u.number == 0)))

#define CHECK_TYPES(val, t, arg, inst) \
  if (!((val)->type & (t))) bad_argument(val, t, arg, inst);

/* Beek - add some sanity to joining strings */
/* add to an svalue */
#define EXTEND_SVALUE_STRING(x, y, z) \
    SAFE( char *ess_res; \
        int ess_len; \
        int ess_r; \
        ess_len = (ess_r = SVALUE_STRLEN(x)) + strlen(y); \
        if (ess_len > MAX_STRING_LENGTH) \
            error("Maximum string length exceeded in concatenation.\n"); \
        if ((x)->subtype == STRING_MALLOC && MSTR_REF((x)->u.string) == 1) { \
            ess_res = (char *) extend_string((x)->u.string, ess_len); \
            if (!ess_res) fatal("Out of memory!\n"); \
            strcpy(ess_res + ess_r, (y)); \
        } else { \
            ess_res = new_string(ess_len, z); \
            strcpy(ess_res, (x)->u.string); \
            strcpy(ess_res + ess_r, (y)); \
            free_string_svalue(x); \
            (x)->subtype = STRING_MALLOC; \
        } \
        (x)->u.string = ess_res; \
    )

/* <something that needs no free> + string svalue */
#define SVALUE_STRING_ADD_LEFT(y, z) \
    SAFE( char *pss_res; int pss_r; int pss_len; \
        pss_len = SVALUE_STRLEN(sp) + (pss_r = strlen(y)); \
        if (pss_len > MAX_STRING_LENGTH) \
            error("Maximum string length exceeded in concatenation.\n"); \
        pss_res = new_string(pss_len, z); \
        strcpy(pss_res, y); \
        strcpy(pss_res + pss_r, sp->u.string); \
        free_string_svalue(sp--); \
        sp->type = T_STRING; \
        sp->u.string = pss_res; \
        sp->subtype = STRING_MALLOC; \
     )

/* basically, string + string; faster than using extend b/c of SVALUE_STRLEN */
#define SVALUE_STRING_JOIN(x, y, z) \
    SAFE( char *ssj_res; int ssj_r; int ssj_len; \
        ssj_r = SVALUE_STRLEN(x); \
        ssj_len = ssj_r + SVALUE_STRLEN(y); \
        if (ssj_len > MAX_STRING_LENGTH) \
            error("Maximum string length exceeded in concatenation.\n"); \
        if ((x)->subtype == STRING_MALLOC && MSTR_REF((x)->u.string) == 1) { \
            ssj_res = (char *) extend_string((x)->u.string, ssj_len); \
            if (!ssj_res) fatal("Out of memory!\n"); \
            (void) strcpy(ssj_res + ssj_r, (y)->u.string);	\
            free_string_svalue(y); \
        } else { \
            ssj_res = (char *) new_string(ssj_len, z); \
            strcpy(ssj_res, (x)->u.string);	       \
            strcpy(ssj_res + ssj_r, (y)->u.string);	       \
            free_string_svalue(y); \
            free_string_svalue(x); \
            (x)->subtype = STRING_MALLOC; \
        } \
        (x)->u.string = ssj_res; \
    )

/* macro calls */
#define call_program(prog, offset) \
        eval_instruction(prog->program + offset)

#ifdef DEBUG
#define free_svalue(x,y) int_free_svalue(x,y)
#else
#define free_svalue(x,y) int_free_svalue(x)
#endif

#define CHECK_STACK_OVERFLOW(x) if (sp + (x) >= end_of_stack) SAFE( too_deep_error = 1; error("stack overflow"); )
#define STACK_INC SAFE( CHECK_STACK_OVERFLOW(1); sp++; )

#define push_svalue(x) SAFE( \
                            STACK_INC;\
                            assign_svalue_no_free(sp, x);\
                            )
#define put_number(x) SAFE( \
                           sp->type = T_NUMBER;\
                           sp->subtype = 0;\
                           sp->u.number = (x);\
                           )
#define put_buffer(x) SAFE( \
                           sp->type = T_BUFFER;\
                           sp->u.buf = (x);\
                           )
#define put_undested_object(x) SAFE(\
                                    sp->type = T_OBJECT;\
                                    sp->u.ob = (x);\
                                    )
#define put_object(x) SAFE(\
                           if (!(x) || (x)->flags & O_DESTRUCTED) *sp = const0u;\
                           else put_undested_object(x);\
                           )
#define put_unrefed_undested_object(x, y) SAFE(\
                                               sp->type = T_OBJECT;\
                                               sp->u.ob = (x);\
                                               add_ref((x), y);\
                                               )
#define put_unrefed_object(x,y) SAFE(\
                                     if (!(x) || (x)->flags & O_DESTRUCTED) *sp = const0u;\
                                     else put_unrefed_undested_object(x,y);\
                                     )
/* see comments on push_constant_string */
#define put_constant_string(x) SAFE(\
                                    sp->type = T_STRING;\
                                    sp->subtype = STRING_SHARED;\
                                    sp->u.string = make_shared_string(x);\
                                    )
#define put_malloced_string(x) SAFE(\
                                    sp->type = T_STRING;\
                                    sp->subtype = STRING_MALLOC;\
                                    sp->u.string = (x);\
                                    )
#define put_array(x) SAFE(\
                          sp->type = T_ARRAY;\
                          sp->u.arr = (x);\
                          )
#define put_shared_string(x) SAFE(\
                                  sp->type = T_STRING;\
                                  sp->subtype = STRING_SHARED;\
                                  sp->u.string = (x);\
                                  )

#define FOREACH_LEFT_GLOBAL 1
#define FOREACH_RIGHT_GLOBAL 2
#define FOREACH_REF 4
#define FOREACH_MAPPING 8

extern program_t *current_prog;
extern short caller_type;
extern char *pc;
extern svalue_t *sp;
extern svalue_t *fp;
extern svalue_t *end_of_stack;
extern svalue_t catch_value;
extern control_stack_t control_stack[CFG_MAX_CALL_DEPTH+5];
extern control_stack_t *csp;
extern int too_deep_error;
extern int max_eval_error;
extern int function_index_offset;
extern int variable_index_offset;
extern unsigned int apply_low_call_others;
extern unsigned int apply_low_cache_hits;
extern unsigned int apply_low_slots_used;
extern unsigned int apply_low_collisions;
extern int simul_efun_is_loading;
extern program_t fake_prog;
extern svalue_t global_lvalue_byte;
extern int num_varargs;
extern int st_num_arg;
extern svalue_t const0;
extern svalue_t const1;
extern svalue_t const0u;
extern svalue_t apply_ret_value;
extern ref_t *global_ref_list;
extern int lv_owner_type;
extern refed_t *lv_owner;

void kill_ref (ref_t *);
ref_t *make_ref (void);

void init_interpreter (void);
void call_direct (object_t *, int, int, int);
void eval_instruction (char *p);
INLINE void assign_svalue (svalue_t *, svalue_t *);
INLINE void assign_svalue_no_free (svalue_t *, svalue_t *);
INLINE void copy_some_svalues (svalue_t *, svalue_t *, int);
INLINE void transfer_push_some_svalues (svalue_t *, int);
INLINE void push_some_svalues (svalue_t *, int);
#ifdef DEBUG
INLINE void int_free_svalue (svalue_t *, const char *);
#else
INLINE void int_free_svalue (svalue_t *);
#endif
INLINE void free_string_svalue (svalue_t *);
INLINE void free_some_svalues (svalue_t *, int);
INLINE void push_object (object_t *);
INLINE void push_number (long);
INLINE void push_real (double);
INLINE void push_undefined (void);
INLINE void copy_and_push_string (const char *);
INLINE void share_and_push_string (const char *);
INLINE void push_array (array_t *);
INLINE void push_refed_array (array_t *);
#ifndef NO_BUFFER_TYPE
INLINE void push_buffer (buffer_t *);
INLINE void push_refed_buffer (buffer_t *);
#endif
INLINE void push_mapping (mapping_t *);
INLINE void push_refed_mapping (mapping_t *);
INLINE void push_class (array_t *);
INLINE void push_refed_class (array_t *);
INLINE void push_malloced_string (const char *);
INLINE void push_shared_string (const char *);
INLINE void push_constant_string (const char *);
INLINE void pop_stack (void);
INLINE void pop_n_elems (int);
INLINE void pop_2_elems (void);
INLINE void pop_3_elems (void);
INLINE function_t *setup_inherited_frame (int);
INLINE program_t *find_function_by_name (object_t *, const char *, int *, int *);
char *function_name (program_t *, int);
void remove_object_from_stack (object_t *);
void setup_fake_frame (funptr_t *);
void remove_fake_frame (void);
void push_indexed_lvalue (int);
void setup_variables (int, int, int);

void process_efun_callback (int, function_to_call_t *, int);
svalue_t *call_efun_callback (function_to_call_t *, int);
svalue_t *safe_call_efun_callback (function_to_call_t *, int);
const char *type_name (int c);
void bad_arg (int, int);
void bad_argument (svalue_t *, int, int, int);
void check_for_destr (array_t *);
int is_static (const char *, object_t *);
int apply_low (const char *, object_t *, int);
svalue_t *apply (const char *, object_t *, int, int);
svalue_t *call_function_pointer (funptr_t *, int);
svalue_t *safe_call_function_pointer (funptr_t *, int);
svalue_t *safe_apply (const char *, object_t *, int, int);
void call___INIT (object_t *);
array_t *call_all_other (array_t *, const char *, int);
const char *function_exists (const char *, object_t *, int);
void call_function (program_t *, int);
void mark_apply_low_cache (void);
void translate_absolute_line (int, unsigned short *, int *, int *);
char *add_slash (const char * const);
int strpref (const char *, const char *);
array_t *get_svalue_trace (void);
void do_trace (const char *, const char *, const char *);
const char *dump_trace (int);
void opcdump (const char *);
int inter_sscanf (svalue_t *, svalue_t *, svalue_t *, int);
char * get_line_number_if_any (void);
char *get_line_number (char *, const program_t *);
void get_line_number_info (const char **, int *);
void get_version (char *);
void reset_machine (int);
void unlink_string_svalue (svalue_t *);
void copy_lvalue_range (svalue_t *);
void assign_lvalue_range (svalue_t *);
void debug_perror (const char *, const char *);



#ifndef NO_SHADOWS
int validate_shadowing (object_t *);
#endif

#if !defined(NO_RESETS) && defined(LAZY_RESETS)
void try_reset (object_t *);
#endif

void pop_context (error_context_t *);
void restore_context (error_context_t *);
int save_context (error_context_t *);

void pop_control_stack (void);
INLINE function_t *setup_new_frame (int);
INLINE void push_control_stack (int);

void break_point (void);

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_svalue (svalue_t *);
void mark_stack (void);
#endif

#endif                          /* _INTERPRET_H */
