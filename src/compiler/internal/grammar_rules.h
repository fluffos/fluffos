#include "vm/internal/base/number.h"

struct argument_t {
  short num_arg;
  char flags;
};

void rule_program(struct parse_node_t* node);
bool rule_inheritence(struct parse_node_t** $$, int $1, char* $3);

LPC_INT rule_func_type(LPC_INT $1, LPC_INT $2, char* $3);
LPC_INT rule_func_proto(LPC_INT $1, LPC_INT $2, char** $3, argument_t $5);
void rule_func(parse_node_t** $$, LPC_INT $1, LPC_INT $2, char* $3, argument_t $5, LPC_INT* $8,
               parse_node_t** $9);
struct ident_hash_elem_t* rule_define_class(LPC_INT* $$, char* $3);
void rule_define_class_members(struct ident_hash_elem_t* $2, LPC_INT $5);
