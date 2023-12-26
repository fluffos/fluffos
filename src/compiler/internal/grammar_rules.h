#include "vm/internal/base/number.h"

// in compiler.h
struct argument_t;

void rule_program(struct parse_node_t* node);
bool rule_inheritence(struct parse_node_t** $$, int $1, char* $3);

LPC_INT rule_func_type(LPC_INT type, LPC_INT optional_star, char* identifier);
LPC_INT rule_func_proto(LPC_INT, LPC_INT, char**, argument_t);
void rule_func(parse_node_t** function, LPC_INT type, LPC_INT optional_star, char* identifier, argument_t argument, LPC_INT* func_types,
               parse_node_t** block_or_semi);
struct ident_hash_elem_t* rule_define_class(LPC_INT* $$, char* $3);
void rule_define_class_members(struct ident_hash_elem_t* $2, LPC_INT $5);
