/* interpret.h */

#ifndef INTERPRET_H
#define INTERPRET_H

#define PUSH_STRING    (0 << 6)
#define PUSH_NUMBER    (1 << 6)
#define PUSH_GLOBAL    (2 << 6)
#define PUSH_LOCAL     (3 << 6)

#define PUSH_WHAT      (3 << 6)
#define PUSH_MASK      (0xff ^ (PUSH_WHAT))

#define SWITCH_CASE_SIZE ((int)(2 + sizeof(char *)))

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
#  ifdef LPC_TO_C
#    define TRACE_COMPILED 256
#    define TRACE_LPC_EXEC 512
#  endif
#  define TRACETST(b) (command_giver->interactive->trace_level & (b))
#  define TRACEP(b) \
    (command_giver && command_giver->interactive && TRACETST(b) && \
     (command_giver->interactive->trace_prefix == 0 || \
      (current_object && strpref(command_giver->interactive->trace_prefix, \
	      current_object->name))) )
#  define TRACEHB (current_heart_beat == 0 || (command_giver->interactive->trace_level & TRACE_HEART_BEAT))
#endif

#ifdef HAS_UNSIGNED_CHAR
#define EXTRACT_UCHAR(p) (*(unsigned char *)(p))
#else
#define EXTRACT_UCHAR(p) (*p < 0 ? *p + 0x100 : *p)
#endif				/* HAS_UNSIGNED_CHAR */

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

typedef struct {
#ifdef PROFILE_FUNCTIONS
    unsigned long entry_secs, entry_usecs;
#endif
    short framekind;
    union {
	function_t *func;
	funptr_t *funp;
    } fr;
    object_t *ob;		/* Current object */
    object_t *prev_ob;	/* Save previous object */
    program_t *prog;	/* Current program */
    int num_local_variables;	/* Local + arguments */
    char *pc;
    svalue_t *fp;
    int function_index_offset;	/* Used when executing functions in inherited
				 * programs */
    int variable_index_offset;	/* Same */
    short caller_type;		/* was this a locally called function? */
} control_stack_t;

typedef struct error_context_s {
    jmp_buf context;
    control_stack_t *save_csp;
    object_t *save_command_giver; 
    svalue_t *save_sp;
    struct error_context_s *save_context;
} error_context_t;

/* for apply_master_ob */
#define MASTER_APPROVED(x) (((x)==(svalue_t *)-1) || ((x) && (((x)->type != T_NUMBER) || (x)->u.number))) 

#define IS_ZERO(x) (!(x) || (((x)->type == T_NUMBER) && ((x)->u.number == 0)))
#define IS_UNDEFINED(x) (!(x) || (((x)->type == T_NUMBER) && \
	((x)->subtype == T_UNDEFINED) && ((x)->u.number == 0)))
#define IS_NULL(x) (!(x) || (((x)->type == T_NUMBER) && \
	((x)->subtype == T_NULLVALUE) && ((x)->u.number == 0)))

#define CHECK_TYPES(val, t, arg, inst) \
  if (!((val)->type & (t))) bad_argument(val, t, arg, inst);

/* Beek - add some sanity to joining strings */
/* add to an svalue */
#define EXTEND_SVALUE_STRING(x, y, z) \
    SAFE( char *ess_res; \
      int ess_len; \
      int ess_r; \
      ess_len = (ess_r = SVALUE_STRLEN(x)) + strlen(y); \
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
      (x)->u.string = ess_res;  )

/* <something that needs no free> + string svalue */
#define SVALUE_STRING_ADD_LEFT(y, z) \
    SAFE( char *pss_res; int pss_r; int pss_len; \
        pss_len = SVALUE_STRLEN(sp) + (pss_r = strlen(y)); \
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
        if ((x)->subtype == STRING_MALLOC && MSTR_REF((x)->u.string) == 1) { \
            ssj_res = (char *) extend_string((x)->u.string, ssj_len); \
            if (!ssj_res) fatal("Out of memory!\n"); \
            (void) strcpy(ssj_res + ssj_r, (y)->u.string); \
            free_string_svalue(y); \
        } else { \
            ssj_res = (char *) new_string(ssj_len, z); \
	    strcpy(ssj_res, (x)->u.string); \
	    strcpy(ssj_res + ssj_r, (y)->u.string); \
	    free_string_svalue(y); \
            free_string_svalue(x); \
            (x)->subtype = STRING_MALLOC; \
        } \
        (x)->u.string = ssj_res; \
    )

/* macro calls */
#ifndef LPC_TO_C
#define call_program(prog, offset) \
        eval_instruction(prog->program + offset)
#endif

#ifdef DEBUG
#define free_svalue(x,y) int_free_svalue(x,y)
#else
#define free_svalue(x,y) int_free_svalue(x)
#endif

#define push_svalue(x) assign_svalue_no_free(++sp, x)
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
			   if ((x)->flags & O_DESTRUCTED) put_number(0); \
			   else put_undested_object(x);\
			   )
#define put_unrefed_undested_object(x, y) SAFE(\
					       sp->type = T_OBJECT;\
					       sp->u.ob = (x);\
					       add_ref((x), y);\
					       )
#define put_unrefed_object(x,y) SAFE(\
				     if ((x)->flags & O_DESTRUCTED)\
				     put_number(0);\
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

extern program_t *current_prog;
extern short caller_type;
extern char *pc;
extern svalue_t *sp;
extern svalue_t *fp;
extern svalue_t catch_value;
extern control_stack_t control_stack[MAX_TRACE];
extern control_stack_t *csp;
extern int too_deep_error;
extern int max_eval_error;
extern int function_index_offset;
extern int variable_index_offset;
extern unsigned int apply_low_call_others;
extern unsigned int apply_low_cache_hits;
extern unsigned int apply_low_slots_used;
extern unsigned int apply_low_collisions;
extern int function_index_offset;
extern int master_ob_is_loading;
extern int simul_efun_is_loading;
extern program_t fake_prog;
extern svalue_t global_lvalue_byte;
extern int num_varargs;

/* with LPC_TO_C off, these are defines using eval_instruction */
#ifdef LPC_TO_C
void call_program PROT((program_t *, POINTER_INT));
#endif
void eval_instruction PROT((char *p));
INLINE void assign_svalue PROT((svalue_t *, svalue_t *));
INLINE void assign_svalue_no_free PROT((svalue_t *, svalue_t *));
INLINE void copy_some_svalues PROT((svalue_t *, svalue_t *, int));
INLINE void transfer_push_some_svalues PROT((svalue_t *, int));
INLINE void push_some_svalues PROT((svalue_t *, int));
#ifdef DEBUG
INLINE void int_free_svalue PROT((svalue_t *, char *));
#else
INLINE void int_free_svalue PROT((svalue_t *));
#endif
INLINE void free_string_svalue PROT((svalue_t *));
INLINE void free_some_svalues PROT((svalue_t *, int));
INLINE void push_object PROT((object_t *));
INLINE void push_number PROT((int));
INLINE void push_real PROT((double));
INLINE void push_undefined PROT((void));
INLINE void push_null PROT((void));
INLINE void push_string PROT((char *, int));
INLINE void push_array PROT((array_t *));
INLINE void push_refed_array PROT((array_t *));
INLINE void push_buffer PROT((buffer_t *));
INLINE void push_refed_buffer PROT((buffer_t *));
INLINE void push_mapping PROT((mapping_t *));
INLINE void push_refed_mapping PROT((mapping_t *));
INLINE void push_class PROT((array_t *));
INLINE void push_refed_class PROT((array_t *));
INLINE void push_malloced_string PROT((char *));
INLINE void push_constant_string PROT((char *));
INLINE void pop_stack PROT((void));
INLINE void pop_n_elems PROT((int));
INLINE void pop_2_elems PROT((void));
INLINE void pop_3_elems PROT((void));
INLINE function_t *setup_inherited_frame PROT((function_t *));
void remove_object_from_stack PROT((object_t *));
void setup_fake_frame PROT((funptr_t *));
void remove_fake_frame PROT((void));
void push_indexed_lvalue PROT((int));

char *type_name PROT((int c));
void bad_arg PROT((int, int));
void bad_argument PROT((svalue_t *, int, int, int));
void check_for_destr PROT((array_t *));
int is_static PROT((char *, object_t *));
int apply_low PROT((char *, object_t *, int));
svalue_t *apply PROT((char *, object_t *, int, int));
svalue_t *call_function_pointer PROT((funptr_t *, int));
svalue_t *safe_call_function_pointer PROT((funptr_t *, int));
svalue_t *safe_apply PROT((char *, object_t *, int, int));
void call___INIT PROT((object_t *));
array_t *call_all_other PROT((array_t *, char *, int));
char *function_exists PROT((char *, object_t *));
void call_function PROT((program_t *, function_t *));
svalue_t *apply_master_ob PROT((char *, int));
svalue_t *safe_apply_master_ob PROT((char *, int));
int assert_master_ob_loaded PROT((char *, char *));
void mark_apply_low_cache PROT((void));

void translate_absolute_line PROT((int, unsigned short *, int *, int *));
char *add_slash PROT((char *));
int strpref PROT((char *, char *));
array_t *get_svalue_trace PROT((void));
void do_trace PROT((char *, char *, char *));
char *dump_trace PROT((int));
void opcdump PROT((char *));
int inter_sscanf PROT((svalue_t *, svalue_t *, svalue_t *, int));
char * get_line_number_if_any PROT((void));
char *get_line_number PROT((char *, program_t *));
void get_line_number_info PROT((char **, int *));
void get_version PROT((char *));
void reset_machine PROT((int));
void unlink_string_svalue PROT((svalue_t *));
void copy_lvalue_range PROT((svalue_t *));
void assign_lvalue_range PROT((svalue_t *));

#ifndef NO_SHADOWS
int validate_shadowing PROT((object_t *));
#endif

#ifdef LAZY_RESETS
void try_reset PROT((object_t *));
#endif

void pop_context PROT((error_context_t *));
void restore_context PROT((error_context_t *));
void save_context PROT((error_context_t *));

void pop_control_stack PROT((void));
INLINE function_t *setup_new_frame PROT((function_t *));
INLINE void push_control_stack PROT((int, void *));

void break_point PROT((void));

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_svalue PROT((svalue_t *));
void mark_stack PROT((void));
#endif

#endif				/* _INTERPRET_H */
