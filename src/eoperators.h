/* this file was manually generated -- don't delete it */

INLINE void f_ge PROT((void));
INLINE void f_le PROT((void));
INLINE void f_lt PROT((void));
INLINE void f_gt PROT((void));
INLINE void f_and PROT((void));
INLINE void f_and_eq PROT((void));
INLINE void f_div_eq PROT((void));
INLINE void f_eq PROT((void));
INLINE void f_lsh PROT((void));
INLINE void f_lsh_eq PROT((void));
INLINE void f_mod_eq PROT((void));
INLINE void f_mult_eq PROT((void));
INLINE void f_ne PROT((void));
INLINE void f_or PROT((void));
INLINE void f_or_eq PROT((void));
INLINE void f_parse_command PROT((void));
INLINE void f_range PROT((int));
INLINE void f_extract_range PROT((int));
INLINE void f_rsh PROT((void));
INLINE void f_rsh_eq PROT((void));
INLINE void f_simul_efun PROT((void));
INLINE void f_sub_eq PROT((void));
INLINE void f_switch PROT((void));
INLINE void f_xor PROT((void));
INLINE void f_xor_eq PROT((void));
INLINE void f_function_constructor PROT((void));
INLINE void f_evaluate PROT((void));
INLINE void f_sscanf PROT((void));

/*
 * eoperators.c
 */
INLINE funptr_t *make_funp PROT((svalue_t *, svalue_t *));
INLINE void push_funp PROT((funptr_t *));
INLINE void free_funp PROT((funptr_t *));
int merge_arg_lists PROT((int, array_t *, int));
void call_simul_efun PROT((unsigned short, int));

INLINE funptr_t *make_efun_funp PROT((int, svalue_t *));
INLINE funptr_t *make_lfun_funp PROT((int, svalue_t *));
INLINE funptr_t *make_simul_funp PROT((int, svalue_t *));
