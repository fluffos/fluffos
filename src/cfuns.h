#ifndef CFUNS_H
#define CFUNS_H

#ifdef LPC_TO_C
typedef struct {
    short index1, index2;
} range_switch_entry_t;

typedef struct string_switch_entry_s {
    char *string;
    int index;
} string_switch_entry_t;

void mark_switch_lists PROT((void));

extern int lpc_int;
extern svalue_t *lpc_svp;
extern array_t *lpc_arr;
extern mapping_t *lpc_map;

void c_return PROT((void));
void c_return_zero PROT((void));
void c_call_inherited PROT((int, int, int));
void c_call PROT((int, int));
void c_efun_return PROT((int));
void c_void_assign PROT((void));
void c_post_dec PROT((void));
void c_post_inc PROT((void));
void c_pre_dec PROT((void));
void c_pre_inc PROT((void));
void c_assign PROT((void));
void c_void_assign_local PROT((svalue_t *));
void c_index PROT((void));
void c_rindex PROT((void));
void c_function_constructor PROT((int, int));
void c_not PROT((void));
void c_mod PROT((void));
void c_add_eq PROT((int));
void c_divide PROT((void));
void c_multiply PROT((void));
void c_inc PROT((void));
void c_dec PROT((void));
void c_le PROT((void));
void c_lt PROT((void));
void c_gt PROT((void));
void c_ge PROT((void));
void c_subtract PROT((void));
void c_evaluate PROT((int));
void c_negate PROT((void));
void c_compl PROT((void));
void c_add PROT((void));
int c_loop_cond_compare PROT((svalue_t *, svalue_t *));
void c_sscanf PROT((int));
void c_parse_command PROT((int));
void c_prepare_catch PROT((error_context_t *));
void c_caught_error PROT((error_context_t *));
void c_end_catch PROT((error_context_t *));
void c_functional PROT((int, int, POINTER_INT));
void fix_switches PROT((string_switch_entry_t **));
int c_string_switch_lookup PROT((svalue_t *, string_switch_entry_t *, int));
int c_range_switch_lookup PROT((int, range_switch_entry_t *, int));
void c_foreach PROT((int, int, int));
int c_next_foreach PROT((void));
void c_exit_foreach PROT((void));
void c_anonymous PROT((int, int, POINTER_INT));
void c_expand_varargs PROT((int));
void c_new_class PROT((int, int));
void c_member_lvalue PROT((int));
void c_member PROT((int));

#endif

#endif

