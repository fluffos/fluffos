#include "base/std.h"

#include "vm/vm.h"
#include "vm/internal/base/machine.h"
#include "compiler/internal/compiler.h"
#include "compiler/internal/lexer.h"
#include "compiler/internal/scratchpad.h"
#include "compiler/internal/generate.h"
#include "compiler/internal/grammar_rules.h"

#include <fmt/format.h>

extern int context;
extern int func_present;
extern int num_refs;

void rule_switch_pre(LPC_INT* saved_context, LPC_INT* saved_cases_size) {
  *saved_context = context;
  context &= LOOP_CONTEXT;
  context |= SWITCH_CONTEXT;
  *saved_cases_size = mem_block[A_CASES].current_size;
}

void rule_switch(parse_node_t** result, parse_node_t* expr, decl_t* local_decls,
                 parse_node_t* case_node, parse_node_t* switch_block_node, LPC_INT saved_context,
                 LPC_INT saved_cases_size) {
  parse_node_t *cases_list, *switch_node;

  if (switch_block_node) {
    CREATE_STATEMENTS(cases_list, case_node, switch_block_node);
  } else {
    cases_list = case_node;
  }

  if (context & SWITCH_STRINGS) {
    NODE_NO_LINE(switch_node, NODE_SWITCH_STRINGS);
  } else if (context & SWITCH_RANGES) {
    NODE_NO_LINE(switch_node, NODE_SWITCH_RANGES);
  } else if ((context & SWITCH_NUMBERS) || (context & SWITCH_NOT_EMPTY)) {
    NODE_NO_LINE(switch_node, NODE_SWITCH_NUMBERS);
  } else {
    NODE_NO_LINE(switch_node, NODE_SWITCH_NUMBERS);
    yyerror("need case statements in switch/case, not just default:");
  }

  switch_node->l.expr = expr;
  switch_node->r.expr = cases_list;
  prepare_cases(switch_node, saved_cases_size);
  context = saved_context;
  *result = switch_node;
  pop_n_locals(local_decls->num);
}

void rule_case_single(parse_node_t** result, parse_node_t* label) {
  *result = label;
  (*result)->v.expr = 0;
  add_to_mem_block(A_CASES, (char*)&label, sizeof(label));
}

void rule_case_range(parse_node_t** result, parse_node_t* label1, parse_node_t* label2) {
  if (label1->kind != NODE_CASE_NUMBER || label2->kind != NODE_CASE_NUMBER) {
    yyerror("String case labels not allowed as range bounds");
  }
  if (label1->r.number > label2->r.number) {
    return;
  }

  context |= SWITCH_RANGES;
  *result = label1;
  (*result)->v.expr = label2;
  add_to_mem_block(A_CASES, (char*)&label1, sizeof(label1));
}

void rule_case_range_from(parse_node_t** result, parse_node_t* label1) {
  if (label1->kind != NODE_CASE_NUMBER) {
    yyerror("String case labels not allowed as range bounds");
  }

  context |= SWITCH_RANGES;
  *result = label1;
  (*result)->v.expr = new_node();
  (*result)->v.expr->kind = NODE_CASE_NUMBER;
  (*result)->v.expr->r.number = LPC_INT_MAX;
  add_to_mem_block(A_CASES, (char*)&label1, sizeof(label1));
}

void rule_case_range_to(parse_node_t** result, parse_node_t* label2) {
  if (label2->kind != NODE_CASE_NUMBER) {
    yyerror("String case labels not allowed as range bounds");
  }

  context |= SWITCH_RANGES;
  *result = new_node();
  (*result)->kind = NODE_CASE_NUMBER;
  (*result)->r.number = LPC_INT_MIN;
  (*result)->v.expr = label2;
  add_to_mem_block(A_CASES, (char*)result, sizeof(*result));
}

void rule_case_default(parse_node_t** result) {
  if (context & SWITCH_DEFAULT) {
    yyerror("Duplicate default label");
  }
  *result = new_node();
  (*result)->kind = NODE_DEFAULT;
  (*result)->v.expr = 0;
  add_to_mem_block(A_CASES, (char*)result, sizeof(*result));
  context |= SWITCH_DEFAULT;
}

void rule_case_label_constant(parse_node_t** result, LPC_INT val) {
  if ((context & SWITCH_STRINGS) && val) {
    yyerror("Mixed case label list not allowed");
  }

  if (val) {
    context |= SWITCH_NUMBERS;
  } else {
    context |= SWITCH_NOT_EMPTY;
  }

  *result = new_node();
  (*result)->kind = NODE_CASE_NUMBER;
  (*result)->r.number = val;
}

void rule_case_label_string(parse_node_t** result, const ScratchString* str) {
  POINTER_INT p_str = store_prog_string(str);

  if (context & SWITCH_NUMBERS) {
    yyerror("Mixed case label list not allowed");
  }
  context |= SWITCH_STRINGS;

  *result = new_node();
  (*result)->kind = NODE_CASE_STRING;
  (*result)->r.number = (LPC_INT)p_str;
}

void rule_constant_or(LPC_INT* result, LPC_INT val1, LPC_INT val2) { *result = val1 | val2; }

void rule_constant_xor(LPC_INT* result, LPC_INT val1, LPC_INT val2) { *result = val1 ^ val2; }

void rule_constant_and(LPC_INT* result, LPC_INT val1, LPC_INT val2) { *result = val1 & val2; }

void rule_constant_eq_ne(LPC_INT* result, LPC_INT op, LPC_INT val1, LPC_INT val2) {
  *result = (op == F_EQ) ? (val1 == val2) : (val1 != val2);
}

void rule_constant_order(LPC_INT* result, LPC_INT val1, LPC_INT op, LPC_INT val2) {
  switch (op) {
    case F_GE:
      *result = val1 >= val2;
      break;
    case F_LE:
      *result = val1 <= val2;
      break;
    case F_GT:
      *result = val1 > val2;
      break;
    default:
      *result = 0;
      break;
  }
}

void rule_constant_lt(LPC_INT* result, LPC_INT val1, LPC_INT val2) { *result = val1 < val2; }

void rule_constant_shift(LPC_INT* result, LPC_INT op, LPC_INT val1, LPC_INT val2) {
  *result = (op == F_LSH) ? (val1 << val2) : (val1 >> val2);
}

void rule_constant_add(LPC_INT* result, LPC_INT val1, LPC_INT val2) { *result = val1 + val2; }

void rule_constant_sub(LPC_INT* result, LPC_INT val1, LPC_INT val2) { *result = val1 - val2; }

void rule_constant_mul(LPC_INT* result, LPC_INT val1, LPC_INT val2) { *result = val1 * val2; }

void rule_constant_mod(LPC_INT* result, LPC_INT val1, LPC_INT val2) {
  if (val2) {
    *result = val1 % val2;
  } else {
    yyerror("Modulo by zero");
    *result = 0;
  }
}

void rule_constant_div(LPC_INT* result, LPC_INT val1, LPC_INT val2) {
  if (val2) {
    *result = val1 / val2;
  } else {
    yyerror("Division by zero");
    *result = 0;
  }
}

void rule_constant_neg(LPC_INT* result, LPC_INT val) { *result = -val; }

void rule_constant_not(LPC_INT* result, LPC_INT val) { *result = !val; }

void rule_constant_compl(LPC_INT* result, LPC_INT val) { *result = ~val; }

void rule_switch_block_case(parse_node_t** result, parse_node_t* case_node,
                            parse_node_t* block_node) {
  if (block_node) {
    CREATE_STATEMENTS(*result, case_node, block_node);
  } else {
    *result = case_node;
  }
}

void rule_switch_block_stmt(parse_node_t** result, parse_node_t* stmt_node,
                            parse_node_t* block_node) {
  if (block_node) {
    CREATE_STATEMENTS(*result, stmt_node, block_node);
  } else {
    *result = stmt_node;
  }
}

void rule_switch_block_empty(parse_node_t** result) { *result = nullptr; }
