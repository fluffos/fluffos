/*
 * Defines and types for LPC stackmachine code
 */

#ifndef _ICODE_H
#define _ICODE_H
#include "trees.h"

void i_generate___INIT PROT((void));
void i_generate_node PROT((parse_node_t *));
void i_generate_continue PROT((void));
void i_generate_forward_jump PROT((void));
void i_update_forward_jump PROT((void));
void i_update_continues PROT((void));
void i_branch_backwards PROT((char, int));
void i_update_breaks PROT((void));
void i_save_loop_info PROT((parse_node_t *));
void i_restore_loop_info PROT((void));
void i_generate_forward_branch PROT((char));
void i_update_forward_branch PROT((void));
void i_update_forward_branch_links PROT((char, parse_node_t *));
void i_generate_else PROT((void));
void i_initialize_parser PROT((void));
void i_generate_final_program PROT((int));
void i_generate_inherited_init_call PROT((int, short));

void optimize_icode PROT((char *, char *, char *));
#endif
