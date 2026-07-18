#ifndef _GENERATE_H
#define _GENERATE_H

#include "vm/internal/base/program.h"  // for ADDRESS_TYPE

#define generate_function_call i_generate_function_call
#define generate_inherited_init_call i_generate_inherited_init_call
#define generate___INIT i_generate___INIT
#define generate_final_program i_generate_final_program
#define initialize_parser i_initialize_parser
#define uninitialize_parser i_uninitialize_parser

int node_always_true(struct parse_node_t*);
ADDRESS_TYPE generate(struct parse_node_t*);
ADDRESS_TYPE generate_function(struct function_t*, struct parse_node_t*, int);
int generate_conditional_branch(struct parse_node_t*);

// Tree dumping is a product feature now (lpcc --ast), not debug
// scaffolding -- unconditional in every build type.
void dump_tree(struct parse_node_t*);
// lpcc --ast --json: one-line {"fluffos_lpcc":1,"stage":"ast",...} envelope
// covering both trees, with per-node source lines.
void dump_program_ast_json(const char* filename, struct parse_node_t* tree_main,
                           struct parse_node_t* tree_init);
void lpc_tree_form(struct parse_node_t*, struct parse_node_t*);

#endif
