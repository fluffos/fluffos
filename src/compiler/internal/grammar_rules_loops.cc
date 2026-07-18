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

void rule_while(parse_node_t** result, parse_node_t* expr, parse_node_t* stmt,
                LPC_INT saved_context) {
  CREATE_LOOP(*result, 1, stmt, 0, optimize_loop_test(expr));
  context = saved_context;
}

void rule_do(parse_node_t** result, parse_node_t* stmt, parse_node_t* expr, LPC_INT saved_context) {
  CREATE_LOOP(*result, 0, stmt, 0, optimize_loop_test(expr));
  context = saved_context;
}

void rule_cond(parse_node_t** result, parse_node_t* expr, parse_node_t* then_branch,
               parse_node_t* else_branch) {
  /* x != 0 -> x */
  if (IS_NODE(expr, NODE_BINARY_OP, F_NE)) {
    if (IS_NODE(expr->r.expr, NODE_NUMBER, 0))
      expr = expr->l.expr;
    else if (IS_NODE(expr->l.expr, NODE_NUMBER, 0))
      expr = expr->r.expr;
  }

  if (then_branch == 0) {
    if (else_branch == 0) {
      /* if (x) ; -> x; */
      *result = pop_value(expr);
      return;
    } else {
      /* if (x) {} else y; -> if (!x) y; */
      parse_node_t* repl;
      CREATE_UNARY_OP(repl, F_NOT, TYPE_NUMBER, expr);
      expr = repl;
      then_branch = else_branch;
      else_branch = 0;
    }
  }
  CREATE_IF(*result, expr, then_branch, else_branch);
}

void rule_foreach_var_defined(decl_t* result, ident_hash_elem_t* ihe) {
  if (ihe->dn.local_num != -1) {
    CREATE_OPCODE_1(result->node, F_LOCAL_LVALUE, 0, ihe->dn.local_num);
    type_of_locals_ptr[ihe->dn.local_num] &= ~LOCAL_MOD_UNUSED;
  } else if (ihe->dn.global_num != -1) {
    CREATE_OPCODE_1(result->node, F_GLOBAL_LVALUE, 0, ihe->dn.global_num);
  } else {
    char buf[256];
    char* end = EndOf(buf);
    char* p;

    p = strput(buf, end, "'");
    p = strput(p, end, ihe->name);
    p = strput(p, end, "' is not a local or a global variable.");
    yyerror(buf);
    CREATE_OPCODE_1(result->node, F_GLOBAL_LVALUE, 0, 0);
  }
  result->num = 0;
}

void rule_foreach_var_new_local(decl_t* result, LPC_INT local_num) {
  if (type_of_locals_ptr[local_num] & LOCAL_MOD_REF) {
    CREATE_OPCODE_1(result->node, F_REF_LVALUE, 0, local_num);
  } else {
    CREATE_OPCODE_1(result->node, F_LOCAL_LVALUE, 0, local_num);
    type_of_locals_ptr[local_num] &= ~LOCAL_MOD_UNUSED;
  }
  result->num = 1;
}

void rule_foreach_var_identifier(decl_t* result, const ScratchString* identifier) {
  yyerror("'%s' is not a local or a global variable.", identifier->c_str());
  CREATE_OPCODE_1(result->node, F_GLOBAL_LVALUE, 0, 0);
  result->num = 0;
}

void rule_foreach_vars_single(decl_t* result, decl_t* var) {
  CREATE_FOREACH(result->node, var->node, 0);
  result->num = var->num;
}

void rule_foreach_vars_double(decl_t* result, decl_t* var1, decl_t* var2) {
  CREATE_FOREACH(result->node, var1->node, var2->node);
  result->num = var1->num + var2->num;
  if (var1->node->v.number == F_REF_LVALUE) {
    yyerror("Mapping key may not be a reference in foreach()");
  }
}

LPC_INT rule_foreach_open() {
  LPC_INT saved = context;
  context = LOOP_CONTEXT | LOOP_FOREACH;
  return saved;
}

void rule_foreach(decl_t* result, decl_t* vars, parse_node_t* expr, parse_node_t* stmt,
                  LPC_INT saved_context) {
  vars->node->v.expr = expr;
  result->num = vars->num;
  CREATE_STATEMENTS(result->node, vars->node, 0);
  CREATE_LOOP(result->node->r.expr, 2, stmt, 0, 0);
  CREATE_OPCODE(result->node->r.expr->r.expr, F_NEXT_FOREACH, 0);
  context = saved_context;
}

void rule_for_init_expr(decl_t* result, parse_node_t* expr) {
  result->node = expr;
  result->num = 0;
}

void rule_for_init_local(decl_t* result, parse_node_t* expr) {
  result->node = expr;
  result->num = 1;
}

void rule_for(decl_t* result, decl_t* first, parse_node_t* cond, parse_node_t* incr,
              parse_node_t* stmt, LPC_INT saved_context) {
  first->node = pop_value(first->node);
  result->num = first->num;
  incr = pop_value(incr);
  if (incr && IS_NODE(incr, NODE_UNARY_OP, F_INC) &&
      IS_NODE(incr->r.expr, NODE_OPCODE_1, F_LOCAL_LVALUE)) {
    LPC_INT lvar = incr->r.expr->l.number;
    CREATE_OPCODE_1(incr, F_LOOP_INCR, 0, lvar);
  }

  CREATE_STATEMENTS(result->node, first->node, 0);
  CREATE_LOOP(result->node->r.expr, 1, stmt, incr, optimize_loop_test(cond));
  context = saved_context;
}

parse_node_t* rule_statement_expr(parse_node_t* expr) {
  parse_node_t* res = pop_value(expr);
#ifdef DEBUG
  {
    parse_node_t* replacement;
    CREATE_STATEMENTS(replacement, res, 0);
    CREATE_OPCODE(replacement->r.expr, F_BREAK_POINT, 0);
    res = replacement;
  }
#endif
  return res;
}

parse_node_t* rule_statement_compound_stmt(decl_t decl_val) {
  parse_node_t* node = decl_val.node;
  pop_n_locals(decl_val.num);
  return node;
}

parse_node_t* rule_statement_break() {
  parse_node_t* node;
  if (context & SPECIAL_CONTEXT) {
    yyerror("Cannot break out of catch { } or time_expression { }");
    node = 0;
  } else if (context & SWITCH_CONTEXT) {
    CREATE_CONTROL_JUMP(node, CJ_BREAK_SWITCH);
  } else if (context & LOOP_CONTEXT) {
    CREATE_CONTROL_JUMP(node, CJ_BREAK);
    if (context & LOOP_FOREACH) {
      parse_node_t* replace;
      CREATE_STATEMENTS(replace, 0, node);
      CREATE_OPCODE(replace->l.expr, F_EXIT_FOREACH, 0);
      node = replace;
    }
  } else {
    yyerror("break statement outside loop");
    node = 0;
  }
  return node;
}

parse_node_t* rule_statement_continue() {
  parse_node_t* node;
  if (context & SPECIAL_CONTEXT)
    yyerror("Cannot continue out of catch { } or time_expression { }");
  else if (!(context & LOOP_CONTEXT))
    yyerror("continue statement outside loop");
  CREATE_CONTROL_JUMP(node, CJ_CONTINUE);
  return node;
}

void rule_return_void(parse_node_t** result) {
  if (exact_types && !IS_TYPE(exact_types, TYPE_VOID))
    yywarn("Non-void functions must return a value.");
  CREATE_RETURN(*result, 0);
  // A real, user-written `return;` is reduced while current_line is still
  // on the statement itself (unlike the implicit-return synthesis in
  // rule_func()/rule_primary_expr_anon_func(), which fires after the whole
  // body -- and possibly an #include pop -- has been consumed). Stamp it
  // explicitly: CREATE_RETURN stays new_node_no_line() by default because
  // a bare `return <constant-or-nothing>;` can otherwise be the ONLY node
  // in a statement's whole codegen (every child is itself a
  // new_node_no_line() leaf -- CREATE_NUMBER/CREATE_REAL/CREATE_STRING),
  // so switch_to_line() would never fire for it and its bytecode would
  // silently inherit whatever line was active beforehand (abs_line 0 if
  // it's the first statement generated in the compile unit).
  (*result)->line = current_line_base + current_line;
}

void rule_return_expr(parse_node_t** result, parse_node_t* expr) {
  if (exact_types && !compatible_types(expr->type, exact_types)) {
    char buf[256];
    char* end = EndOf(buf);
    char* p;

    p = strput(buf, end, "Type of returned value doesn't match function return type ");
    p = get_two_types(p, end, expr->type, exact_types);
    yyerror(buf);
  }
  if (IS_NODE(expr, NODE_NUMBER, 0)) {
    CREATE_RETURN(*result, 0);
  } else {
    CREATE_RETURN(*result, expr);
  }
  // See the comment in rule_return_void() above.
  (*result)->line = current_line_base + current_line;
}
