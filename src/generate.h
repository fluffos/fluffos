#ifndef _GENERATE_H
#define _GENERATE_H

#include "lint.h"
#include "trees.h"

void generate_expr PROT((struct parse_node *));
void generate_function_call PROT((short, char));
void pop_value PROT((void));
void generate_return PROT((struct parse_node *));
void generate_break_point PROT((void));
void generate_break PROT((void));
void generate_continue PROT((void));
void generate_forward_jump PROT((void));
void update_forward_jump PROT((void));
void update_continues PROT((void));
void save_position PROT((void));
void branch_backwards PROT((char));
void update_breaks PROT((void));
void save_loop_info PROT((void));
void restore_loop_info PROT((void));
void start_switch PROT((void));
void generate_forward_branch PROT((char));
void update_forward_branch PROT((void));
void generate_else PROT((void));
void initialize_parser PROT((void));
void generate_conditional_branch PROT((struct parse_node *));
int node_always_true PROT((struct parse_node *));
void generate_return PROT((struct parse_node *));
void generate___INIT PROT((void));
void generate_final_program PROT((int));

#ifdef DEBUG
void dump_expr_list PROT((struct parse_node *, int));
void dump_tree PROT((struct parse_node *, int));
#endif

#endif
