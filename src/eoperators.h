/* this file was manually generated -- don't delete it */

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
#ifndef NEW_FUNCTIONS
INLINE void f_evaluate PROT((void));
#endif
INLINE void f_sscanf PROT((void));

/*
 * eoperators.c
 */
INLINE struct funp *make_funp PROT((struct svalue *, struct svalue *));
INLINE void push_funp PROT((struct funp *));
INLINE void free_funp PROT((struct funp *));
int merge_arg_lists PROT((int, struct vector *, int));
void call_simul_efun PROT((unsigned short, int));

