#ifndef _GENERATE_H
#define _GENERATE_H

#include "lint.h"
#include "trees.h"
#include "program.h"
#include "icode.h"

#ifdef LPC_TO_C
#include "ccode.h"

#define generate_function_call(x,y) { if(compile_to_c) c_generate_function_call(x,y); else i_generate_function_call(x,y); }
#define generate_inherited_init_call(x,y) { if (compile_to_c) c_generate_inherited_init_call(x,y); else i_generate_inherited_init_call(x,y); }
#define generate___INIT() { if (compile_to_c) c_generate___INIT(); else i_generate___INIT(); }
#define generate_final_program(x) { if (compile_to_c) c_generate_final_program(x); else i_generate_final_program(x); }
#define initialize_parser() { if (compile_to_c) c_initialize_parser(); else i_initialize_parser(); }
#else
#define generate_function_call i_generate_function_call
#define generate_inherited_init_call i_generate_inherited_init_call
#define generate___INIT i_generate___INIT
#define generate_final_program i_generate_final_program
#define initialize_parser i_initialize_parser
#endif

int node_always_true PROT((parse_node_t *));
short generate PROT((parse_node_t *));
short generate_function PROT((compiler_function_t *, parse_node_t *, int));
int generate_conditional_branch PROT((parse_node_t *));

#ifdef DEBUG
void dump_expr_list PROT((parse_node_t *));
void dump_tree PROT((parse_node_t *));
void lpc_tree_form PROT((parse_node_t *, parse_node_t *));
#endif

#endif
