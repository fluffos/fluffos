#ifndef CCODE_H
#define CCODE_H

#include "program.h"

void c_start_function PROT((function_t *));
void c_end_function PROT((void));

void c_generate_inherited_call PROT((int, short));
void c_generate_final_program PROT((int));
void c_generate___INIT PROT((void));
void c_initialize_parser PROT((void));
void c_generate_node PROT((parse_node_t *));
void c_analyze PROT((parse_node_t *));
void c_generate_inherited_init_call PROT((int, short));

#endif
