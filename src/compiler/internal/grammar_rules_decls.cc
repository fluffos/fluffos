#include "base/std.h"

#include "vm/vm.h"
#include "vm/internal/base/machine.h"
#include "compiler/internal/compiler.h"
#include "compiler/internal/lex.h"
#include "compiler/internal/scratchpad.h"
#include "compiler/internal/generate.h"
#include "compiler/internal/grammar_rules.h"

#include <fmt/format.h>

extern int context;
extern int func_present;
extern int num_refs;
extern char *outp;

struct parse_node_t *rule_block_or_semi(struct parse_node_t *block_node) {
  if (!block_node) {
    CREATE_RETURN(block_node, 0);
  }
  return block_node;
}

void rule_def_global_var(LPC_INT type_val) {
  if (!(type_val & ~(DECL_MODS)) && (pragmas & PRAGMA_STRICT_TYPES)) {
    yyerror("Missing type for global variable declaration");
  }
}

char *rule_new_local_name_redefine(ident_hash_elem_t *ihe) {
  if (ihe->dn.local_num != -1) {
    yyerror("Illegal to redeclare local name '%s'", ihe->name);
  }
  return scratch_copy(ihe->name);
}

void rule_new_name(LPC_INT star_modifier, char *identifier) {
  if (current_type & (FUNC_VARARGS << 16)) {
    yyerror("Illegal to declare varargs variable.");
    current_type &= ~(FUNC_VARARGS << 16);
  }

  if (current_type & 0xffff0000) {
    current_type = (current_type >> 16) | (current_type & 0xffff);
  }

  current_type |= global_modifiers;

  if (!(current_type & DECL_ACCESS)) current_type |= DECL_PUBLIC;

  if ((current_type & ~DECL_MODS) == TYPE_VOID)
    yyerror("Illegal to declare global variable of type void.");

  define_new_variable(identifier, current_type | star_modifier);
  scratch_free(identifier);
}

void rule_new_name_with_init(LPC_INT star_modifier, char *identifier, LPC_INT assign_val, parse_node_t *expr) {
  parse_node_t *expr_node, *newnode;
  int type;

  if (current_type & (FUNC_VARARGS << 16)) {
    yyerror("Illegal to declare varargs variable.");
    current_type &= ~(FUNC_VARARGS << 16);
  }

  if (current_type & 0xffff0000) {
    current_type = (current_type >> 16) | (current_type & 0xffff);
  }

  current_type |= global_modifiers;

  if (!(current_type & DECL_ACCESS)) current_type |= DECL_PUBLIC;

  if ((current_type & ~DECL_MODS) == TYPE_VOID)
    yyerror("Illegal to declare global variable of type void.");

  if (assign_val != F_ASSIGN)
    yyerror("Only '=' is legal in initializers.");

  if (current_type) {
    type = (current_type | star_modifier) & ~DECL_MODS;
    if ((current_type & ~DECL_MODS) == TYPE_VOID)
      yyerror("Illegal to declare global variable of type void.");
    if (!compatible_types(type, expr->type)) {
      char buff[256];
      char *end = EndOf(buff);
      char *p;

      p = strput(buff, end, "Type mismatch ");
      p = get_two_types(p, end, type, expr->type);
      p = strput(p, end, " when initializing ");
      p = strput(p, end, identifier);
      yyerror(buff);
    }
  } else type = 0;
  expr = do_promotions(expr, type);

  CREATE_BINARY_OP(expr_node, F_VOID_ASSIGN, 0, expr, 0);
  CREATE_OPCODE_1(expr_node->r.expr, F_GLOBAL_LVALUE, 0,
      define_new_variable(identifier, current_type | star_modifier));
  newnode = comp_trees[TREE_INIT];
  CREATE_TWO_VALUES(comp_trees[TREE_INIT], 0,
      newnode, expr_node);
  scratch_free(identifier);
}

void rule_block(decl_t *result, parse_node_t *stmts_node, int entry_locals) {
  result->node = stmts_node;
  result->num = current_number_of_locals - entry_locals;
}

parse_node_t *rule_new_local_def(char *name, LPC_INT type_star) {
  if (current_type & LOCAL_MOD_REF) {
    yyerror("Illegal to declare local variable as reference");
    current_type &= ~LOCAL_MOD_REF;
  }
  add_local_name(name, current_type | type_star | LOCAL_MOD_UNUSED);
  scratch_free(name);
  return nullptr;
}

parse_node_t *rule_new_local_def_with_init(char *name, LPC_INT type_star, LPC_INT assign_op, parse_node_t *expr) {
  int type = (current_type | type_star) & ~DECL_MODS;

  if (current_type & LOCAL_MOD_REF) {
    yyerror("Illegal to declare local variable as reference");
    current_type &= ~LOCAL_MOD_REF;
    type &= ~LOCAL_MOD_REF;
  }

  if (assign_op != F_ASSIGN)
    yyerror("Only '=' is allowed in initializers.");
  if (!compatible_types(expr->type, type)) {
    char buff[256];
    char *end = EndOf(buff);
    char *p;

    p = strput(buff, end, "Type mismatch ");
    p = get_two_types(p, end, type, expr->type);
    p = strput(p, end, " when initializing ");
    p = strput(p, end, name);
    yyerror(buff);
  }

  expr = do_promotions(expr, type);

  parse_node_t *res;
  CREATE_UNARY_OP_1(res, F_VOID_ASSIGN_LOCAL, 0, expr,
      add_local_name(name, current_type | type_star | LOCAL_MOD_UNUSED));
  scratch_free(name);
  return res;
}

parse_node_t *rule_single_new_local_def_with_init(LPC_INT local_num, LPC_INT assign_op, parse_node_t *expr) {
  int type = type_of_locals_ptr[local_num];

  if (type & LOCAL_MOD_REF) {
    yyerror("Illegal to declare local variable as reference");
    type_of_locals_ptr[local_num] &= ~LOCAL_MOD_REF;
  }
  type &= ~LOCAL_MODS;

  if (assign_op != F_ASSIGN)
    yyerror("Only '=' is allowed in initializers.");
  if (!compatible_types(expr->type, type)) {
    char buff[256];
    char *end = EndOf(buff);
    char *p;

    p = strput(buff, end, "Type mismatch ");
    p = get_two_types(p, end, type, expr->type);
    p = strput(p, end, " when initializing.");
    yyerror(buff);
  }

  expr = do_promotions(expr, type);

  parse_node_t *res;
  CREATE_BINARY_OP(res, F_ASSIGN, 0, expr, 0);
  CREATE_OPCODE_1(res->r.expr, F_LOCAL_LVALUE, 0, local_num);
  return res;
}

void rule_local_declarations_set_type(LPC_INT basic_type) {
  if (basic_type == TYPE_VOID)
    yyerror("Illegal to declare local variable of type void.");
  current_type = basic_type;
}

void rule_local_declarations(decl_t *result, decl_t *decl1, decl_t *decl2) {
  if (decl1->node && decl2->node) {
    CREATE_STATEMENTS(result->node, decl1->node, decl2->node);
  } else {
    result->node = (decl1->node ? decl1->node : decl2->node);
  }
  result->num = decl1->num + decl2->num;
}

void rule_block_statements_empty(decl_t *result) {
  result->node = 0;
  result->num = 0;
}

void rule_block_statements_stmt(decl_t *result, parse_node_t *stmt, decl_t *stmts) {
  if (stmt && stmts->node) {
    CREATE_STATEMENTS(result->node, stmt, stmts->node);
  } else {
    result->node = (stmt ? stmt : stmts->node);
  }
  result->num = stmts->num;
}

void rule_block_statements_decl(decl_t *result, decl_t *decl_stmt, decl_t *stmts) {
  if (decl_stmt->node && stmts->node) {
    CREATE_STATEMENTS(result->node, decl_stmt->node, stmts->node);
  } else {
    result->node = (decl_stmt->node ? decl_stmt->node : stmts->node);
  }
  result->num = decl_stmt->num + stmts->num;
}

void rule_block_statements_error(decl_t *result, decl_t *stmts) {
  result->node = stmts->node;
  result->num = stmts->num;
}

void rule_local_declaration_statement_set_type(LPC_INT basic_type) {
  if (basic_type == TYPE_VOID)
    yyerror("Illegal to declare local variable of type void.");
  current_type = basic_type;
}

void rule_local_declaration_statement(decl_t *result, decl_t *decl_list) {
  result->node = decl_list->node;
  result->num = decl_list->num;
}

void rule_local_name_list_single(decl_t *result, parse_node_t *node) {
  result->node = node;
  result->num = 1;
}

void rule_local_name_list_multi(decl_t *result, parse_node_t *node, decl_t *list) {
  if (node && list->node) {
    CREATE_STATEMENTS(result->node, node, list->node);
  } else {
    result->node = (node ? node : list->node);
  }
  result->num = 1 + list->num;
}
