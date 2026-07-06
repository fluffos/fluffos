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

parse_node_t *rule_expr_or_block_block(decl_t decl_val) {
  return decl_val.node;
}

parse_node_t *rule_expr_or_block_expr(parse_node_t *expr) {
  return insert_pop_value(expr);
}

void rule_catch(parse_node_t **result, parse_node_t *expr_or_block, LPC_INT saved_context) {
  CREATE_CATCH(*result, expr_or_block);
  context = saved_context;
}

void rule_sscanf(parse_node_t **result, parse_node_t *expr1, parse_node_t *expr2, parse_node_t *lvalue_list) {
  int p = lvalue_list->v.number;
  CREATE_LVALUE_EFUN(*result, TYPE_NUMBER, lvalue_list);
  CREATE_BINARY_OP_1((*result)->l.expr, F_SSCANF, 0, expr1, expr2, p);
}

void rule_parse_command(parse_node_t **result, parse_node_t *expr1, parse_node_t *expr2, parse_node_t *expr3, parse_node_t *lvalue_list) {
  int p = lvalue_list->v.number;
  CREATE_LVALUE_EFUN(*result, TYPE_NUMBER, lvalue_list);
  CREATE_TERNARY_OP_1((*result)->l.expr, F_PARSE_COMMAND, 0, expr1, expr2, expr3, p);
}

void rule_time_expression(parse_node_t **result, parse_node_t *expr_or_block, LPC_INT saved_context) {
  CREATE_TIME_EXPRESSION(*result, expr_or_block);
  context = saved_context;
}

parse_node_t *rule_lvalue_list_empty() {
  parse_node_t *node = new_node_no_line();
  node->r.expr = 0;
  node->v.number = 0;
  return node;
}

parse_node_t *rule_lvalue_list(parse_node_t *lvalue, parse_node_t *list) {
  parse_node_t *insert = new_node_no_line();
  insert->r.expr = list->r.expr;
  insert->l.expr = lvalue;
  list->r.expr = insert;
  list->v.number++;
  return list;
}

void rule_string(parse_node_t **result, char *str) {
  CREATE_STRING(*result, str);
  scratch_free(str);
}

void rule_string_like_concat(parse_node_t **result, parse_node_t *left, parse_node_t *right) {
  CREATE_BINARY_OP(*result, F_ADD, TYPE_STRING, left, right);
}

void rule_template_literal(parse_node_t **result, char *head, parse_node_t *expr, parse_node_t *rest) {
  parse_node_t *head_node, *coerced, *tmp;
  CREATE_STRING(head_node, head);
  scratch_free(head);
  CREATE_UNARY_OP(coerced, F_TEMPLATE_COERCE, TYPE_STRING, expr);
  CREATE_BINARY_OP(tmp, F_ADD, TYPE_STRING, head_node, coerced);
  CREATE_BINARY_OP(*result, F_ADD, TYPE_STRING, tmp, rest);
}

void rule_template_parts_tail(parse_node_t **result, char *tail) {
  CREATE_STRING(*result, tail);
  scratch_free(tail);
}

void rule_template_parts_middle(parse_node_t **result, char *mid, parse_node_t *expr, parse_node_t *rest) {
  parse_node_t *mid_node, *coerced, *tmp;
  CREATE_STRING(mid_node, mid);
  scratch_free(mid);
  CREATE_UNARY_OP(coerced, F_TEMPLATE_COERCE, TYPE_STRING, expr);
  CREATE_BINARY_OP(tmp, F_ADD, TYPE_STRING, mid_node, coerced);
  CREATE_BINARY_OP(*result, F_ADD, TYPE_STRING, tmp, rest);
}

parse_node_t *rule_class_init(char *identifier, parse_node_t *expr) {
  parse_node_t *node = new_node();
  node->l.expr = (parse_node_t *)identifier;
  node->v.expr = expr;
  node->r.expr = 0;
  return node;
}

parse_node_t *rule_opt_class_init_empty() {
  return 0;
}

parse_node_t *rule_opt_class_init(parse_node_t *list, parse_node_t *class_init) {
  class_init->r.expr = list;
  return class_init;
}

LPC_INT rule_efun_override(char *identifier) {
  LPC_INT res;
  ident_hash_elem_t *ihe;

  res = (ihe = lookup_ident(identifier)) ? ihe->dn.efun_num : -1;
  if (res == -1) {
    yyerror("Unknown efun: %s", identifier);
  } else {
    push_malloced_string(the_file_name(current_file));
    share_and_push_string(identifier);
    push_malloced_string(add_slash(main_file_name()));
    svalue_t *ret = safe_apply_master_ob(APPLY_VALID_OVERRIDE, 3);
    if (!MASTER_APPROVED(ret)) {
      yyerror("Invalid simulated efunction override");
      res = -1;
    }
  }
  scratch_free(identifier);
  return res;
}

LPC_INT rule_efun_override_new() {
  push_malloced_string(the_file_name(current_file));
  push_constant_string("new");
  push_malloced_string(add_slash(main_file_name()));
  svalue_t *res = safe_apply_master_ob(APPLY_VALID_OVERRIDE, 3);
  if (!MASTER_APPROVED(res)) {
    yyerror("Invalid simulated efunction override");
    return -1;
  } else {
    return new_efun;
  }
}

char *rule_function_name_colon_colon(char *identifier) {
  int l = strlen(identifier) + 1;
  char *p;
  char *res = scratch_realloc(identifier, l + 3);
  p = res + l;
  while (p--, l--)
    *(p + 3) = *p;
  strncpy(res, ":::", 3);
  return res;
}

char *rule_function_name_type(LPC_INT basic_type, char *identifier) {
  int z, l = strlen(identifier) + 1;
  char *p;
  z = strlen(compiler_type_names[basic_type]) + 3;
  char *res = scratch_realloc(identifier, l + z);
  p = res + l;
  while (p--, l--)
    *(p + z) = *p;
  res[0] = ':';
  strncpy(res + 1, compiler_type_names[basic_type], z - 3);
  res[z - 2] = ':';
  res[z - 1] = ':';
  return res;
}

char *rule_function_name_obj(char *obj, char *identifier) {
  int l = strlen(obj);
  char *res = scratch_alloc(l + strlen(identifier) + 4);
  *(res) = ':';
  strcpy(res + 1, obj);
  strcpy(res + l + 1, "::");
  strcpy(res + l + 3, identifier);
  scratch_free(obj);
  scratch_free(identifier);
  return res;
}

LPC_INT rule_functional_open(LPC_INT val) {
  return (val << 8) | FP_EFUN;
}

parse_node_t *rule_lvalue(parse_node_t *expr) {
#define LV_ILLEGAL 1
#define LV_RANGE 2
#define LV_INDEX 4
  parse_node_t *res = expr;
  if (res->kind == NODE_BINARY_OP && res->v.number == F_TYPE_CHECK)
    res = res->l.expr;
  switch (res->kind) {
    default:
      yyerror("Illegal lvalue");
      break;
    case NODE_PARAMETER:
      res->kind = NODE_PARAMETER_LVALUE;
      break;
    case NODE_TERNARY_OP:
      res->v.number = res->r.expr->v.number;
      [[fallthrough]];
    case NODE_OPCODE_1:
    case NODE_UNARY_OP_1:
    case NODE_BINARY_OP:
      if (res->v.number >= F_LOCAL && res->v.number <= F_MEMBER)
        res->v.number++;
      else if (res->v.number == F_MAP_MEMBER)
        // Not in the contiguous [F_LOCAL, F_MEMBER] "+1 for lvalue" run --
        // map_member/map_member_lvalue were added later in ops.spec, right
        // after member/member_lvalue but numerically past F_MEMBER. Handled
        // the same way (opcode+1 = lvalue variant), just checked separately.
        res->v.number++;
      else if (res->v.number >= F_INDEX && res->v.number <= F_RE_RANGE) {
        parse_node_t *node = res;
        int flag = 0;
        do {
          switch (node->kind) {
            case NODE_PARAMETER:
              node->kind = NODE_PARAMETER_LVALUE;
              flag |= LV_ILLEGAL;
              break;
            case NODE_TERNARY_OP:
              node->v.number = node->r.expr->v.number;
              [[fallthrough]];
            case NODE_OPCODE_1:
            case NODE_UNARY_OP_1:
            case NODE_BINARY_OP:
              if (node->kind == NODE_BINARY_OP && node->v.number == F_TYPE_CHECK) {
                node = node->l.expr;
                continue;
              }

              if ((node->v.number >= F_LOCAL && node->v.number <= F_MEMBER) ||
                  node->v.number == F_MAP_MEMBER) {
                node->v.number++;
                flag |= LV_ILLEGAL;
                break;
              } else if (node->v.number == F_INDEX || node->v.number == F_RINDEX) {
                node->v.number++;
                flag |= LV_INDEX;
                break;
              } else if (node->v.number >= F_ADD_EQ && node->v.number <= F_ASSIGN) {
                if (!(flag & LV_INDEX)) {
                  yyerror("Illegal lvalue, a possible lvalue is (x <assign> y)[a]");
                }
                if (node->r.expr->kind == NODE_BINARY_OP || node->r.expr->kind == NODE_TERNARY_OP) {
                  if (node->r.expr->v.number >= F_NN_RANGE_LVALUE && node->r.expr->v.number <= F_NR_RANGE_LVALUE)
                    yyerror("Illegal to have (x[a..b] <assign> y) to be the beginning of an lvalue");
                }
                flag = LV_ILLEGAL;
                break;
              } else if (node->v.number >= F_NN_RANGE && node->v.number <= F_RE_RANGE) {
                if (flag & LV_RANGE) {
                  yyerror("Can't do range lvalue of range lvalue.");
                  flag |= LV_ILLEGAL;
                  break;
                }
                if (flag & LV_INDEX) {
                  yyerror("Can't do indexed lvalue of range lvalue.");
                  flag |= LV_ILLEGAL;
                  break;
                }
                if (node->v.number == F_NE_RANGE) {
                  parse_node_t *rchild = node->r.expr;
                  node->kind = NODE_TERNARY_OP;
                  CREATE_BINARY_OP(node->r.expr, F_NR_RANGE_LVALUE, 0, 0, rchild);
                  CREATE_NUMBER(node->r.expr->l.expr, 1);
                } else if (node->v.number == F_RE_RANGE) {
                  parse_node_t *rchild = node->r.expr;
                  node->kind = NODE_TERNARY_OP;
                  CREATE_BINARY_OP(node->r.expr, F_RR_RANGE_LVALUE, 0, 0, rchild);
                  CREATE_NUMBER(node->r.expr->l.expr, 1);
                } else
                  node->r.expr->v.number++;
                flag |= LV_RANGE;
                node = node->r.expr->r.expr;
                continue;
              }
              [[fallthrough]];
            default:
              yyerror("Illegal lvalue");
              flag = LV_ILLEGAL;
              break;
          }
          if ((flag & LV_ILLEGAL) || !(node = node->r.expr)) break;
        } while (1);
        break;
      } else
        yyerror("Illegal lvalue");
      break;
  }
  return res;
#undef LV_ILLEGAL
#undef LV_RANGE
#undef LV_INDEX
}

void rule_expr_ref(parse_node_t **result, parse_node_t *lval) {
  int op;
  if (!(context & ARG_LIST)) {
    yyerror("ref illegal outside function argument list");
    op = 0;
  } else {
    num_refs++;
    switch (lval->kind) {
      case NODE_PARAMETER_LVALUE:
        op = F_LOCAL_LVALUE;
        break;
      case NODE_TERNARY_OP:
      case NODE_OPCODE_1:
      case NODE_UNARY_OP_1:
      case NODE_BINARY_OP:
        op = lval->v.number;
        if (op > F_RINDEX_LVALUE)
          yyerror("Illegal to make reference to range");
        break;
      default:
        op = 0;
        yyerror("unknown lvalue kind");
    }
  }
  CREATE_UNARY_OP_1(*result, F_MAKE_REF, TYPE_ANY, lval, op);
}

void rule_expr_assign(parse_node_t **result, parse_node_t *lval, int opcode, parse_node_t *rval) {
  if (opcode == F_LOR_EQ || opcode == F_LAND_EQ || opcode == F_NULLISH_EQ) {
    if (exact_types && !compatible_types(rval->type, lval->type)) {
      char buf[256];
      char *end = EndOf(buf);
      char *p;
      p = strput(buf, end, "Bad assignment ");
      p = get_two_types(p, end, lval->type, rval->type);
      p = strput(p, end, ".");
      yyerror(buf);
    }
    CREATE_LOGICAL_ASSIGN(*result, opcode, lval, rval);
  } else {
    CREATE_BINARY_OP(*result, opcode, rval->type, rval, lval);

    if (exact_types && !compatible_types(rval->type, lval->type) &&
        !(opcode == F_ADD_EQ && lval->type == TYPE_STRING &&
          ((COMP_TYPE(rval->type, TYPE_NUMBER)) || rval->type == TYPE_OBJECT))) {
      char buf[256];
      char *end = EndOf(buf);
      char *p;
      p = strput(buf, end, "Bad assignment ");
      p = get_two_types(p, end, lval->type, rval->type);
      p = strput(p, end, ".");
      yyerror(buf);
    }

    if (opcode == F_ASSIGN)
      (*result)->l.expr = do_promotions(rval, lval->type);
  }
}

void rule_expr_assign_error(parse_node_t **result, parse_node_t *expr) {
  yyerror("Illegal LHS");
  CREATE_ERROR(*result);
}

void rule_expr_ternary(parse_node_t **result, parse_node_t *cond, parse_node_t *val1, parse_node_t *val2) {
  if (exact_types && !compatible_types2(val1->type, val2->type)) {
    char buf[256];
    char *end = EndOf(buf);
    char *p;

    p = strput(buf, end, "Types in ?: do not match ");
    p = get_two_types(p, end, val1->type, val2->type);
    p = strput(p, end, ".");
    yywarn(buf);
  }

  if (IS_NODE(cond, NODE_UNARY_OP, F_NOT)) {
    CREATE_IF(*result, cond->r.expr, val2, val1);
  } else {
    CREATE_IF(*result, cond, val1, val2);
  }
  (*result)->type = ((val1->type == val2->type) ? val1->type : TYPE_ANY);
}

void rule_primary_expr_cast(parse_node_t **result, LPC_INT type, parse_node_t *expr) {
  *result = expr;
  (*result)->type = type;

  if (exact_types &&
      expr->type != type &&
      expr->type != TYPE_ANY &&
      expr->type != TYPE_UNKNOWN &&
      type != TYPE_VOID) {
    char buf[256];
    char *end = EndOf(buf);
    char *p;

    p = strput(buf, end, "Cannot cast ");
    p = get_type_name(p, end, expr->type);
    p = strput(p, end, "to ");
    p = get_type_name(p, end, type);
    yyerror(buf);
  }
}

void rule_primary_expr_pre_inc(parse_node_t **result, parse_node_t *lval) {
  CREATE_UNARY_OP(*result, F_PRE_INC, 0, lval);
  if (exact_types) {
    switch (lval->type) {
      case TYPE_NUMBER:
      case TYPE_ANY:
      case TYPE_REAL:
        (*result)->type = lval->type;
        break;
      default:
        (*result)->type = TYPE_ANY;
        type_error("Bad argument 1 to ++x", lval->type);
    }
  } else {
    (*result)->type = TYPE_ANY;
  }
}

void rule_primary_expr_pre_dec(parse_node_t **result, parse_node_t *lval) {
  CREATE_UNARY_OP(*result, F_PRE_DEC, 0, lval);
  if (exact_types) {
    switch (lval->type) {
      case TYPE_NUMBER:
      case TYPE_ANY:
      case TYPE_REAL:
        (*result)->type = lval->type;
        break;
      default:
        (*result)->type = TYPE_ANY;
        type_error("Bad argument 1 to --x", lval->type);
    }
  } else {
    (*result)->type = TYPE_ANY;
  }
}

void rule_primary_expr_post_inc(parse_node_t **result, parse_node_t *lval) {
  CREATE_UNARY_OP(*result, F_POST_INC, 0, lval);
  (*result)->v.number = F_POST_INC;
  if (exact_types) {
    switch (lval->type) {
      case TYPE_NUMBER:
      case TYPE_ANY:
      case TYPE_REAL:
        (*result)->type = lval->type;
        break;
      default:
        (*result)->type = TYPE_ANY;
        type_error("Bad argument 1 to x++", lval->type);
    }
  } else {
    (*result)->type = TYPE_ANY;
  }
}

void rule_primary_expr_post_dec(parse_node_t **result, parse_node_t *lval) {
  CREATE_UNARY_OP(*result, F_POST_DEC, 0, lval);
  if (exact_types) {
    switch (lval->type) {
      case TYPE_NUMBER:
      case TYPE_ANY:
      case TYPE_REAL:
        (*result)->type = lval->type;
        break;
      default:
        (*result)->type = TYPE_ANY;
        type_error("Bad argument 1 to x--", lval->type);
    }
  } else {
    (*result)->type = TYPE_ANY;
  }
}

void rule_primary_expr_defined_name(parse_node_t **result, ident_hash_elem_t *ihe) {
  int i;
  if ((i = ihe->dn.local_num) != -1) {
    type_of_locals_ptr[i] &= ~LOCAL_MOD_UNUSED;
    if (type_of_locals_ptr[i] & LOCAL_MOD_REF)
      CREATE_OPCODE_1(*result, F_REF, type_of_locals_ptr[i] & ~LOCAL_MOD_REF, i & 0xff);
    else
      CREATE_OPCODE_1(*result, F_LOCAL, type_of_locals_ptr[i], i & 0xff);
    if (current_function_context)
      current_function_context->num_locals++;
  } else if ((i = ihe->dn.global_num) != -1) {
    if (current_function_context)
      current_function_context->bindable = FP_NOT_BINDABLE;
    CREATE_OPCODE_1(*result, F_GLOBAL, VAR_TEMP(i)->type & ~DECL_MODS, i);
    if (VAR_TEMP(i)->type & DECL_HIDDEN) {
      char buf[256];
      char *end = EndOf(buf);
      char *p;
      p = strput(buf, end, "Illegal to use private variable '");
      p = strput(p, end, ihe->name);
      p = strput(p, end, "'");
      yyerror(buf);
    }
  } else if (ihe->dn.function_num != -1) {
    *result = new_node();
    (*result)->kind = NODE_FUNCTION_CONSTRUCTOR;
    (*result)->type = TYPE_FUNCTION;
    (*result)->r.expr = 0;
    (*result)->l.expr = 0;
    (*result)->v.number = (ihe->dn.function_num << 8) | FP_LOCAL;
    if (current_function_context)
      current_function_context->bindable = FP_NOT_BINDABLE;
  } else if (ihe->dn.simul_num != -1) {
    *result = new_node();
    (*result)->kind = NODE_FUNCTION_CONSTRUCTOR;
    (*result)->type = TYPE_FUNCTION;
    (*result)->r.expr = 0;
    (*result)->l.expr = 0;
    (*result)->v.number = (ihe->dn.simul_num << 8) | FP_SIMUL;
    if (current_function_context)
      current_function_context->bindable = FP_NOT_BINDABLE;
  } else if (ihe->dn.efun_num != -1) {
    *result = new_node();
    (*result)->kind = NODE_FUNCTION_CONSTRUCTOR;
    (*result)->type = TYPE_FUNCTION;
    (*result)->r.expr = 0;
    (*result)->l.expr = 0;
    (*result)->v.number = (ihe->dn.efun_num << 8) | FP_EFUN;
  } else {
    char buf[256];
    char *end = EndOf(buf);
    char *p;
    auto max_local_variables = CFG_INT(__MAX_LOCAL_VARIABLES__);
    p = strput(buf, end, "Undefined variable '");
    p = strput(p, end, ihe->name);
    p = strput(p, end, "'");
    if (current_number_of_locals < max_local_variables) {
      add_local_name(ihe->name, TYPE_ANY);
    }
    CREATE_ERROR(*result);
    yyerror(buf);
  }
}

void rule_primary_expr_identifier(parse_node_t **result, char *name) {
  char buf[256];
  char *end = EndOf(buf);
  char *p;
  auto max_local_variables = CFG_INT(__MAX_LOCAL_VARIABLES__);
  p = strput(buf, end, "Undefined variable '");
  p = strput(p, end, name);
  p = strput(p, end, "'");
  if (current_number_of_locals < max_local_variables) {
    add_local_name(name, TYPE_ANY);
  }
  CREATE_ERROR(*result);
  yyerror(buf);
  scratch_free(name);
}

function_context_t *rule_dollar_open() {
  function_context_t *saved = current_function_context;
  if (current_function_context)
    current_function_context = current_function_context->parent;
  return saved;
}

void rule_primary_expr_dollar_expr(parse_node_t **result, function_context_t *saved_context, parse_node_t *expr) {
  parse_node_t *node;
  current_function_context = saved_context;

  if (!current_function_context || current_function_context->num_parameters < 0) {
    CREATE_ERROR(*result);
  } else {
    CREATE_OPCODE_1(*result, F_LOCAL, expr->type, current_function_context->values_list->kind++);
    node = new_node_no_line();
    node->type = 0;
    current_function_context->values_list->l.expr->r.expr = node;
    current_function_context->values_list->l.expr = node;
    node->r.expr = 0;
    node->v.expr = expr;
  }
}

void rule_primary_expr_member_arrow(parse_node_t **result, parse_node_t *expr, char *identifier) {
  if (expr->type == TYPE_ANY) {
    int cmi;
    unsigned short tp;
    if ((cmi = lookup_any_class_member_soft(identifier, &tp)) != -1) {
      CREATE_UNARY_OP_1(*result, F_MEMBER, tp, expr, 0);
      (*result)->l.number = cmi;
    } else {
      /* Fall back to dynamic lookup (treat like mapping-style access). */
      CREATE_UNARY_OP_1(*result, F_MAP_MEMBER, TYPE_ANY, expr, 0);
      (*result)->l.number = store_prog_string(identifier);
      (*result)->type = TYPE_ANY;
    }
  } else if (!IS_CLASS(expr->type)) {
    yyerror("Left argument of -> is not a class");
    CREATE_ERROR(*result);
  } else {
    CREATE_UNARY_OP_1(*result, F_MEMBER, 0, expr, 0);
    (*result)->l.number = lookup_class_member(CLASS_IDX(expr->type), identifier, &((*result)->type));
  }
  scratch_free(identifier);
}

void rule_primary_expr_member_dot(parse_node_t **result, parse_node_t *expr, char *identifier) {
  if (expr->type == TYPE_ANY) {
    int cmi;
    unsigned short tp;
    if ((cmi = lookup_any_class_member_soft(identifier, &tp)) != -1) {
      CREATE_UNARY_OP_1(*result, F_MEMBER, tp, expr, 0);
      (*result)->l.number = cmi;
    } else {
      /* Fall back to dynamic lookup (treat like mapping-style access). */
      CREATE_UNARY_OP_1(*result, F_MAP_MEMBER, TYPE_ANY, expr, 0);
      (*result)->l.number = store_prog_string(identifier);
      (*result)->type = TYPE_ANY;
    }
  } else if (expr->type == TYPE_MAPPING) {
    CREATE_UNARY_OP_1(*result, F_MAP_MEMBER, TYPE_ANY, expr, 0);
    (*result)->l.number = store_prog_string(identifier);
    (*result)->type = TYPE_ANY;
  } else if (IS_CLASS(expr->type)) {
    CREATE_UNARY_OP_1(*result, F_MEMBER, 0, expr, 0);
    (*result)->l.number = lookup_class_member(CLASS_IDX(expr->type), identifier, &((*result)->type));
  } else {
    /* Default to mapping-style lookup so dynamic mappings still work when
     * the static type isn't known to be one (e.g. still TYPE_ANY-ish
     * despite a concrete non-class, non-mapping declared type). */
    CREATE_UNARY_OP_1(*result, F_MAP_MEMBER, TYPE_ANY, expr, 0);
    (*result)->l.number = store_prog_string(identifier);
    (*result)->type = TYPE_ANY;
  }
  scratch_free(identifier);
}

// Optional chaining member access: `expr?.name`. Mapping-only (unlike '.'/'-'>,
// this never falls back to class-member lookup) -- if expr isn't a mapping at
// runtime, F_MAP_MEMBER_OPTIONAL short-circuits to 0 instead of erroring.
void rule_primary_expr_member_optional(parse_node_t **result, parse_node_t *expr, char *identifier) {
  CREATE_UNARY_OP_1(*result, F_MAP_MEMBER_OPTIONAL, TYPE_ANY, expr, 0);
  (*result)->l.number = store_prog_string(identifier);
  (*result)->type = TYPE_ANY;
  scratch_free(identifier);
}

// Optional chaining bracket index: `expr?.[idx]` or `expr.?[idx]` (both forms
// share this rule -- see the two grammar productions in grammar.y). Like
// rule_primary_expr_member_optional, mapping-only and short-circuits to 0
// instead of erroring when expr isn't a mapping at runtime.
void rule_primary_expr_index_optional(parse_node_t **result, parse_node_t *expr, parse_node_t *idx) {
  CREATE_BINARY_OP(*result, F_MAP_INDEX_OPTIONAL, 0, idx, expr);
  (*result)->type = TYPE_ANY;
}

void rule_primary_expr_range_nn(parse_node_t **result, parse_node_t *expr, parse_node_t *expr1, parse_node_t *expr2) {
  if (!CONFIG_INT(__RC_OLD_RANGE_BEHAVIOR__)) {
    if (CONFIG_INT(__RC_WARN_OLD_RANGE_BEHAVIOR__)) {
      if (expr->type != TYPE_MAPPING && expr2->kind == NODE_NUMBER && expr2->v.number < 0)
        yywarn("A negative constant as the second element of arr[x..y] no longer means indexing from the end.  Use arr[x..<y]");
    }
  }
  *result = make_range_node(F_NN_RANGE, expr, expr1, expr2);
}

void rule_primary_expr_range_rn(parse_node_t **result, parse_node_t *expr, parse_node_t *expr1, parse_node_t *expr2) {
  *result = make_range_node(F_RN_RANGE, expr, expr1, expr2);
}

void rule_primary_expr_range_rr(parse_node_t **result, parse_node_t *expr, parse_node_t *expr1, parse_node_t *expr2) {
  if (expr2->kind == NODE_NUMBER && expr2->v.number <= 1)
    *result = make_range_node(F_RE_RANGE, expr, expr1, 0);
  else
    *result = make_range_node(F_RR_RANGE, expr, expr1, expr2);
}

void rule_primary_expr_range_nr(parse_node_t **result, parse_node_t *expr, parse_node_t *expr1, parse_node_t *expr2) {
  if (expr2->kind == NODE_NUMBER && expr2->v.number <= 1)
    *result = make_range_node(F_NE_RANGE, expr, expr1, 0);
  else
    *result = make_range_node(F_NR_RANGE, expr, expr1, expr2);
}

void rule_primary_expr_range_ne(parse_node_t **result, parse_node_t *expr, parse_node_t *expr1) {
  *result = make_range_node(F_NE_RANGE, expr, expr1, 0);
}

void rule_primary_expr_range_re(parse_node_t **result, parse_node_t *expr, parse_node_t *expr1) {
  *result = make_range_node(F_RE_RANGE, expr, expr1, 0);
}

void rule_primary_expr_index_r(parse_node_t **result, parse_node_t *expr, parse_node_t *idx) {
  if (IS_NODE(expr, NODE_CALL, F_AGGREGATE) && idx->kind == NODE_NUMBER) {
    int i = idx->v.number;
    if (i < 1 || i > expr->l.number)
      yyerror("Illegal index to array constant.");
    else {
      parse_node_t *node = expr->r.expr;
      i = expr->l.number - i;
      while (i--)
        node = node->r.expr;
      *result = node->v.expr;
      return;
    }
  }
  CREATE_BINARY_OP(*result, F_RINDEX, 0, idx, expr);
  if (exact_types) {
    switch (expr->type) {
      case TYPE_MAPPING:
        yyerror("Illegal index for mapping.");
        [[fallthrough]];
      case TYPE_ANY:
        (*result)->type = TYPE_ANY;
        break;
      case TYPE_STRING:
      case TYPE_BUFFER:
        (*result)->type = TYPE_NUMBER;
        if (!IS_TYPE(idx->type, TYPE_NUMBER))
          type_error("Bad type of index", idx->type);
        break;
      default:
        if (expr->type & TYPE_MOD_ARRAY) {
          (*result)->type = expr->type & ~TYPE_MOD_ARRAY;
          if ((*result)->type != TYPE_ANY)
            *result = add_type_check(*result, (*result)->type);
          if (!IS_TYPE(idx->type, TYPE_NUMBER))
            type_error("Bad type of index", idx->type);
        } else {
          type_error("Value indexed has a bad type ", expr->type);
          (*result)->type = TYPE_ANY;
        }
    }
  } else (*result)->type = TYPE_ANY;
}

void rule_primary_expr_index(parse_node_t **result, parse_node_t *expr, parse_node_t *idx) {
  if (IS_NODE(expr, NODE_CALL, F_AGGREGATE) && idx->kind == NODE_NUMBER) {
    int i = idx->v.number;
    if (i < 0 || i >= expr->l.number)
      yyerror("Illegal index to array constant.");
    else {
      parse_node_t *node = expr->r.expr;
      while (i--)
        node = node->r.expr;
      *result = node->v.expr;
      return;
    }
  }
#ifndef OLD_RANGE_BEHAVIOR
  if (idx->kind == NODE_NUMBER && idx->v.number < 0)
    yywarn("A negative constant in arr[x] no longer means indexing from the end.  Use arr[<x]");
#endif
  CREATE_BINARY_OP(*result, F_INDEX, 0, idx, expr);
  if (exact_types) {
    switch (expr->type) {
      case TYPE_MAPPING:
      case TYPE_ANY:
        (*result)->type = TYPE_ANY;
        break;
      case TYPE_STRING:
      case TYPE_BUFFER:
        (*result)->type = TYPE_NUMBER;
        if (!IS_TYPE(idx->type, TYPE_NUMBER))
          type_error("Bad type of index", idx->type);
        break;
      default:
        if (expr->type & TYPE_MOD_ARRAY) {
          (*result)->type = expr->type & ~TYPE_MOD_ARRAY;
          if ((*result)->type != TYPE_ANY)
            *result = add_type_check(*result, (*result)->type);
          if (!IS_TYPE(idx->type, TYPE_NUMBER))
            type_error("Bad type of index", idx->type);
        } else {
          type_error("Value indexed has a bad type ", expr->type);
          (*result)->type = TYPE_ANY;
        }
    }
  } else (*result)->type = TYPE_ANY;
}

void rule_lambda_return_type(func_block_t *saved_block, LPC_INT type) {
  auto max_local_variables = CFG_INT(__MAX_LOCAL_VARIABLES__);
  if (type != TYPE_FUNCTION) yyerror("Reserved type name unexpected.");
  saved_block->num_local = current_number_of_locals;
  saved_block->max_num_locals = max_num_locals;
  saved_block->context = context;
  saved_block->save_current_type = current_type;
  saved_block->save_exact_types = exact_types;
  if (type_of_locals_ptr + max_num_locals + max_local_variables >= &type_of_locals[type_of_locals_size])
    reallocate_locals();
  deactivate_current_locals();
  locals_ptr += current_number_of_locals;
  type_of_locals_ptr += max_num_locals;
  max_num_locals = current_number_of_locals = 0;
  push_function_context();
  current_function_context->num_parameters = -1;
  exact_types = TYPE_ANY;
  context = 0;
}

void rule_primary_expr_anon_func(parse_node_t **result, func_block_t *saved_block, argument_t *arg, decl_t *block) {
  if (arg->flags & ARG_IS_VARARGS) {
    yyerror("Anonymous varargs functions aren't implemented");
  }
  if (!block->node) {
    CREATE_RETURN(block->node, 0);
  } else if (block->node->kind != NODE_RETURN &&
      (block->node->kind != NODE_TWO_VALUES || block->node->r.expr->kind != NODE_RETURN)) {
    parse_node_t *replacement;
    CREATE_STATEMENTS(replacement, block->node, 0);
    CREATE_RETURN(replacement->r.expr, 0);
    block->node = replacement;
  }

  *result = new_node();
  (*result)->kind = NODE_ANON_FUNC;
  (*result)->type = TYPE_FUNCTION;
  (*result)->l.number = (max_num_locals - arg->num_arg);
  (*result)->r.expr = block->node;
  (*result)->v.number = arg->num_arg;
  if (current_function_context->bindable)
    (*result)->v.number |= 0x10000;
  free_all_local_names(1);

  current_number_of_locals = saved_block->num_local;
  max_num_locals = saved_block->max_num_locals;
  context = saved_block->context;
  current_type = saved_block->save_current_type;
  exact_types = saved_block->save_exact_types;
  pop_function_context();

  locals_ptr -= current_number_of_locals;
  type_of_locals_ptr -= max_num_locals;
  reactivate_current_locals();
}

// The name-to-functional-reference encoding, moved from the lexer's
// "(: name" machinery (9.2): locals/globals encode as variable refs
// (functional_1 rejects locals with its own diagnostic, exactly as the
// legacy lexer-encoded path did), functions/simuls/efuns as callables.
// Unknown kinds -- the legacy path silently re-spliced these into an
// anonymous body that then failed to parse -- now get a direct error.
LPC_INT rule_functional_ref(struct ident_hash_elem_t *ihe) {
  int idx;
  if ((idx = ihe->dn.local_num) >= 0) return ((LPC_INT)idx << 8) | FP_L_VAR;
  if ((idx = ihe->dn.global_num) >= 0) return ((LPC_INT)idx << 8) | FP_G_VAR;
  if ((idx = ihe->dn.function_num) >= 0) return ((LPC_INT)idx << 8) | FP_LOCAL;
  if ((idx = ihe->dn.simul_num) >= 0) return ((LPC_INT)idx << 8) | FP_SIMUL;
  if ((idx = ihe->dn.efun_num) >= 0) return ((LPC_INT)idx << 8) | FP_EFUN;
  yyerror("Unknown function '%s' in functional", ihe->name != nullptr ? ihe->name : "?");
  return FP_FUNCTIONAL;
}

void rule_primary_expr_functional_1(parse_node_t **result, LPC_INT val) {
  *result = new_node();
  (*result)->kind = NODE_FUNCTION_CONSTRUCTOR;
  (*result)->type = TYPE_FUNCTION;
  (*result)->r.expr = 0;
  switch (val & 0xff) {
    case FP_L_VAR:
      yyerror("Illegal to use local variable in a functional.");
      CREATE_NUMBER((*result)->l.expr, 0);
      (*result)->l.expr->r.expr = 0;
      (*result)->l.expr->l.expr = 0;
      (*result)->v.number = FP_FUNCTIONAL;
      break;
    case FP_G_VAR:
      CREATE_OPCODE_1((*result)->l.expr, F_GLOBAL, 0, val >> 8);
      (*result)->v.number = FP_FUNCTIONAL | FP_NOT_BINDABLE;
      if (VAR_TEMP((*result)->l.expr->l.number)->type & DECL_HIDDEN) {
        char buf[256];
        char *end = EndOf(buf);
        char *p;
        p = strput(buf, end, "Illegal to use private variable '");
        p = strput(p, end, VAR_TEMP((*result)->l.expr->l.number)->name);
        p = strput(p, end, "'");
        yyerror(buf);
      }
      break;
    default:
      (*result)->v.number = val;
      break;
  }
  // 9.2: the "(:"  lexer glue pushes a function context for EVERY
  // functional form; constructors that don't build a body pop it here
  // (the legacy split pushed only for anon/override and leaked the
  // override's context).
  pop_function_context();
}

void rule_primary_expr_functional_2(parse_node_t **result, LPC_INT val, parse_node_t *opt_arg_list) {
  *result = new_node();
  (*result)->kind = NODE_FUNCTION_CONSTRUCTOR;
  (*result)->type = TYPE_FUNCTION;
  (*result)->v.number = val;
  (*result)->r.expr = opt_arg_list;

  switch (val & 0xff) {
    case FP_EFUN: {
                    int *argp;
                    int f = val >> 8;
                    int num = opt_arg_list->kind;
                    int max_arg = predefs[f].max_args;
                    if (f != -1) {
                      if (num > max_arg && max_arg != -1) {
                        parse_node_t *pn = opt_arg_list;
                        while (pn) {
                          if (pn->type & 1) break;
                          pn = pn->r.expr;
                        }
                        if (!pn) {
                          char bff[256];
                          char *end = EndOf(bff);
                          char *p;
                          p = strput(bff, end, "Too many arguments to ");
                          p = strput(p, end, predefs[f].word);
                          yyerror(bff);
                        }
                      } else if (max_arg != -1 && exact_types) {
                        int i, argn, tmp;
                        parse_node_t *enode = opt_arg_list;
                        argp = &efun_arg_types[predefs[f].arg_index];

                        for (argn = 0; argn < num; argn++) {
                          if (enode->type & 1) break;
                          tmp = enode->v.expr->type;
                          for (i = 0; !compatible_types(tmp, argp[i]) && argp[i] != 0; i++)
                            ;
                          if (argp[i] == 0) {
                            char buf[256];
                            char *end = EndOf(buf);
                            char *p;
                            p = strput(buf, end, "Bad argument ");
                            p = strput_int(p, end, argn + 1);
                            p = strput(p, end, " to efun ");
                            p = strput(p, end, predefs[f].word);
                            p = strput(p, end, "()");
                            yyerror(buf);
                          } else {
                            if (tmp == TYPE_NUMBER && argp[i] == TYPE_REAL) {
                              for (i++; argp[i] && argp[i] != TYPE_NUMBER; i++)
                                ;
                              if (!argp[i])
                                enode->v.expr = promote_to_float(enode->v.expr);
                            }
                            if (tmp == TYPE_REAL && argp[i] == TYPE_NUMBER) {
                              for (i++; argp[i] && argp[i] != TYPE_REAL; i++)
                                ;
                              if (!argp[i])
                                enode->v.expr = promote_to_int(enode->v.expr);
                            }
                          }
                          while (argp[i] != 0)
                            i++;
                          argp += i + 1;
                          enode = enode->r.expr;
                        }
                      }
                    }
                    break;
                  }
    case FP_L_VAR:
    case FP_G_VAR:
                  yyerror("Can't give parameters to functional.");
                  break;
  }
  pop_function_context();  // see functional_1's comment (9.2)
}

void rule_primary_expr_functional_3(parse_node_t **result, parse_node_t *expr) {
  if (current_function_context->num_locals)
    yyerror("Illegal to use local variable in functional.");
  if (current_function_context->values_list->r.expr)
    current_function_context->values_list->r.expr->kind = current_function_context->values_list->kind;

  *result = new_node();
  (*result)->kind = NODE_FUNCTION_CONSTRUCTOR;
  (*result)->type = TYPE_FUNCTION;
  (*result)->l.expr = expr;
  if (expr->kind == NODE_STRING)
    yywarn("Function pointer returning string constant is NOT a function call");
  (*result)->r.expr = current_function_context->values_list->r.expr;
  (*result)->v.number = FP_FUNCTIONAL + current_function_context->bindable
    + (current_function_context->num_parameters << 8);
  pop_function_context();
}

void rule_primary_expr_mapping(parse_node_t **result, parse_node_t *opt_arg_list) {
  CREATE_CALL(*result, F_AGGREGATE_ASSOC, TYPE_MAPPING, opt_arg_list);
}

void rule_primary_expr_array(parse_node_t **result, parse_node_t *opt_arg_list) {
  CREATE_CALL(*result, F_AGGREGATE, TYPE_ANY | TYPE_MOD_ARRAY, opt_arg_list);
}

void rule_call_open(LPC_INT *saved_context, LPC_INT *saved_refs) {
  *saved_context = context;
  *saved_refs = num_refs;
  context |= ARG_LIST;
}

void rule_function_call_efun(parse_node_t **result, LPC_INT efun_idx, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs) {
  context = saved_context;
  *result = validate_efun_call(efun_idx, opt_arg_list);
  *result = check_refs(num_refs - saved_refs, opt_arg_list, *result);
  num_refs = saved_refs;
}

void rule_function_call_new(parse_node_t **result, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs) {
  ident_hash_elem_t *ihe;
  int f;

  context = saved_context;
  ihe = lookup_ident("clone_object");

  if ((f = ihe->dn.simul_num) != -1) {
    *result = opt_arg_list;
    (*result)->kind = NODE_CALL_1;
    (*result)->v.number = F_SIMUL_EFUN;
    (*result)->l.number = f;
    (*result)->type = (SIMUL(f)->type) & ~DECL_MODS;
  } else {
    *result = validate_efun_call(lookup_predef("clone_object"), opt_arg_list);
#ifdef CAST_CALL_OTHERS
    (*result)->type = TYPE_UNKNOWN;
#else
    (*result)->type = TYPE_ANY;
#endif
  }
  *result = check_refs(num_refs - saved_refs, opt_arg_list, *result);
  num_refs = saved_refs;
}

void rule_function_call_new_class(parse_node_t **result, ident_hash_elem_t *ihe, parse_node_t *class_init) {
  parse_node_t *node;

  if (ihe->dn.class_num == -1) {
    char buf[256];
    char *end = EndOf(buf);
    char *p;

    p = strput(buf, end, "Undefined class '");
    p = strput(p, end, ihe->name);
    p = strput(p, end, "'");
    yyerror(buf);
    CREATE_ERROR(*result);
    node = class_init;
    while (node) {
      scratch_free((char *)node->l.expr);
      node = node->r.expr;
    }
  } else {
    int type = ihe->dn.class_num | TYPE_MOD_CLASS;
    if ((node = class_init)) {
      CREATE_TWO_VALUES(*result, type, 0, 0);
      (*result)->l.expr = reorder_class_values(ihe->dn.class_num, node);
      CREATE_OPCODE_1((*result)->r.expr, F_NEW_CLASS, type, ihe->dn.class_num);
    } else {
      CREATE_OPCODE_1(*result, F_NEW_EMPTY_CLASS, type, ihe->dn.class_num);
    }
  }
}

void rule_function_call_new_class_undef(parse_node_t **result, char *name, parse_node_t *class_init) {
  parse_node_t *node;
  char buf[256];
  char *end = EndOf(buf);
  char *p;

  p = strput(buf, end, "Undefined class '");
  p = strput(p, end, name);
  p = strput(p, end, "'");
  yyerror(buf);
  CREATE_ERROR(*result);
  node = class_init;
  while (node) {
    scratch_free((char *)node->l.expr);
    node = node->r.expr;
  }
}

void rule_function_call_defined_name(parse_node_t **result, ident_hash_elem_t *ihe, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs) {
  int f;
  int i;

  context = saved_context;
  *result = opt_arg_list;
  if ((f = ihe->dn.function_num) != -1) {
    if (current_function_context)
      current_function_context->bindable = FP_NOT_BINDABLE;

    (*result)->kind = NODE_CALL_1;
    (*result)->v.number = F_CALL_FUNCTION_BY_ADDRESS;
    (*result)->l.number = f;
    (*result)->type = validate_function_call(f, opt_arg_list->r.expr);
  } else if ((f = ihe->dn.simul_num) != -1) {
    (*result)->kind = NODE_CALL_1;
    (*result)->v.number = F_SIMUL_EFUN;
    (*result)->l.number = f;
    (*result)->type = (SIMUL(f)->type) & ~DECL_MODS;
  } else if ((f = ihe->dn.efun_num) != -1) {
    *result = validate_efun_call(f, opt_arg_list);
  } else if ((i = ihe->dn.local_num) != -1 &&
             (type_of_locals_ptr[i] & ~LOCAL_MODS) == TYPE_FUNCTION) {
    parse_node_t *expr;
    parse_node_t *func_node;
    int local_type = type_of_locals_ptr[i] & ~LOCAL_MODS;
    
    type_of_locals_ptr[i] &= ~LOCAL_MOD_UNUSED;
    
    if (type_of_locals_ptr[i] & LOCAL_MOD_REF)
      CREATE_OPCODE_1(func_node, F_REF, local_type, i & 0xff);
    else
      CREATE_OPCODE_1(func_node, F_LOCAL, local_type, i & 0xff);
    
    (*result)->kind = NODE_EFUN;
    (*result)->l.number = (*result)->v.number + 1;
    (*result)->v.number = predefs[evaluate_efun].token;
#ifdef CAST_CALL_OTHERS
    (*result)->type = TYPE_UNKNOWN;
#else
    (*result)->type = TYPE_ANY;
#endif
    expr = new_node_no_line();
    expr->type = 0;
    expr->v.expr = func_node;
    expr->r.expr = (*result)->r.expr;
    (*result)->r.expr = expr;
    
    if (current_function_context)
      current_function_context->num_locals++;
  } else if ((i = ihe->dn.global_num) != -1 &&
             (VAR_TEMP(i)->type & ~DECL_MODS) == TYPE_FUNCTION) {
    parse_node_t *expr;
    parse_node_t *func_node;
    int global_type = VAR_TEMP(i)->type & ~DECL_MODS;
    
    if (current_function_context)
      current_function_context->bindable = FP_NOT_BINDABLE;
    
    CREATE_OPCODE_1(func_node, F_GLOBAL, global_type, i);
    
    if (VAR_TEMP(i)->type & DECL_HIDDEN) {
      char buf[256];
      char *end = EndOf(buf);
      char *p;
      p = strput(buf, end, "Illegal to use private variable '");
      p = strput(p, end, ihe->name);
      p = strput(p, end, "'");
      yyerror(buf);
    }
    
    (*result)->kind = NODE_EFUN;
    (*result)->l.number = (*result)->v.number + 1;
    (*result)->v.number = predefs[evaluate_efun].token;
#ifdef CAST_CALL_OTHERS
    (*result)->type = TYPE_UNKNOWN;
#else
    (*result)->type = TYPE_ANY;
#endif
    expr = new_node_no_line();
    expr->type = 0;
    expr->v.expr = func_node;
    expr->r.expr = (*result)->r.expr;
    (*result)->r.expr = expr;
  } else {
    if (exact_types) {
      char buf[256];
      char *end = EndOf(buf);
      char *p;
      const char *n = ihe->name;
      if (*n == ':') n++;
      p = strput(buf, end, "Undefined function ");
      p = strput(p, end, n);
      yyerror(buf);
    } else {
      if (current_function_context)
        current_function_context->bindable = FP_NOT_BINDABLE;

      f = define_new_function(ihe->name, 0, 0, DECL_PUBLIC|FUNC_UNDEFINED, TYPE_ANY);
      (*result)->kind = NODE_CALL_1;
      (*result)->v.number = F_CALL_FUNCTION_BY_ADDRESS;
      (*result)->l.number = f;
      (*result)->type = TYPE_ANY;
    }
  }
  *result = check_refs(num_refs - saved_refs, opt_arg_list, *result);
  num_refs = saved_refs;
}

void rule_function_call_name(parse_node_t **result, char *name, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs) {
  context = saved_context;
  *result = opt_arg_list;

  if (current_function_context)
    current_function_context->bindable = FP_NOT_BINDABLE;

  if (*name == ':') {
    int f;
    if ((f = arrange_call_inherited(name + 1, *result)) != -1) {
      ;
    }
  } else {
    int f;
    ident_hash_elem_t *ihe;

    f = (ihe = lookup_ident(name)) ? ihe->dn.function_num : -1;

    if (f == -1) {
      if (exact_types) {
        char buf[256];
        char *end = EndOf(buf);
        char *p;
        char *n = name;
        if (*n == ':') n++;
        p = strput(buf, end, "Undefined function ");
        p = strput(p, end, n);
        yyerror(buf);
      } else {
        f = define_new_function(name, 0, 0, DECL_PUBLIC|FUNC_UNDEFINED, TYPE_ANY);
      }
    }

    if (f != -1) {
      (*result)->kind = NODE_CALL_1;
      (*result)->v.number = F_CALL_FUNCTION_BY_ADDRESS;
      (*result)->l.number = f;
      if (FUNCTION_FLAGS(f) & FUNC_UNDEFINED) {
        (*result)->type = TYPE_ANY;
      } else {
        (*result)->type = validate_function_call(f, opt_arg_list->r.expr);
      }
    }
  }
  *result = check_refs(num_refs - saved_refs, opt_arg_list, *result);
  num_refs = saved_refs;
  scratch_free(name);
}

void rule_function_call_indexed(parse_node_t **result, parse_node_t *expr, parse_node_t *idx, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs) {
  parse_node_t *sub_expr;
  parse_node_t *index_expr;

  context = saved_context;
  *result = opt_arg_list;

  CREATE_BINARY_OP(index_expr, F_INDEX, 0, idx, expr);
  if (exact_types) {
    switch (expr->type) {
      case TYPE_MAPPING:
      case TYPE_ANY:
        index_expr->type = TYPE_ANY;
        break;
      default:
        if (expr->type & TYPE_MOD_ARRAY) {
          index_expr->type = expr->type & ~TYPE_MOD_ARRAY;
        } else {
          index_expr->type = TYPE_ANY;
        }
        break;
    }
  } else {
    index_expr->type = TYPE_ANY;
  }

  (*result)->kind = NODE_EFUN;
  (*result)->l.number = (*result)->v.number + 1;
  (*result)->v.number = predefs[evaluate_efun].token;
#ifdef CAST_CALL_OTHERS
  (*result)->type = TYPE_UNKNOWN;
#else
  (*result)->type = TYPE_ANY;
#endif
  sub_expr = new_node_no_line();
  sub_expr->type = 0;
  sub_expr->v.expr = index_expr;
  sub_expr->r.expr = (*result)->r.expr;
  (*result)->r.expr = sub_expr;
  *result = check_refs(num_refs - saved_refs, opt_arg_list, *result);
  num_refs = saved_refs;
}

void rule_function_call_arrow(parse_node_t **result, parse_node_t *expr, char *identifier, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs) {
  ident_hash_elem_t *ihe;
  int f;
  parse_node_t *pn1, *pn2;

  opt_arg_list->v.number += 2;

  pn1 = new_node_no_line();
  pn1->type = 0;
  pn1->v.expr = expr;
  pn1->kind = opt_arg_list->v.number;

  pn2 = new_node_no_line();
  pn2->type = 0;
  CREATE_STRING(pn2->v.expr, identifier);
  scratch_free(identifier);

  pn2->r.expr = opt_arg_list->r.expr;
  pn1->r.expr = pn2;
  opt_arg_list->r.expr = pn1;

  if (!opt_arg_list->l.expr) opt_arg_list->l.expr = pn2;

  context = saved_context;
  ihe = lookup_ident("call_other");

  if ((f = ihe->dn.simul_num) != -1) {
    *result = opt_arg_list;
    (*result)->kind = NODE_CALL_1;
    (*result)->v.number = F_SIMUL_EFUN;
    (*result)->l.number = f;
    (*result)->type = (SIMUL(f)->type) & ~DECL_MODS;
  } else {
    *result = validate_efun_call(arrow_efun, opt_arg_list);
#ifdef CAST_CALL_OTHERS
    (*result)->type = TYPE_UNKNOWN;
#else
    (*result)->type = TYPE_ANY;
#endif
  }
  *result = check_refs(num_refs - saved_refs, opt_arg_list, *result);
  num_refs = saved_refs;
}

void rule_function_call_star(parse_node_t **result, parse_node_t *expr, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs) {
  parse_node_t *sub_expr;

  context = saved_context;
  *result = opt_arg_list;
  (*result)->kind = NODE_EFUN;
  (*result)->l.number = (*result)->v.number + 1;
  (*result)->v.number = predefs[evaluate_efun].token;
#ifdef CAST_CALL_OTHERS
  (*result)->type = TYPE_UNKNOWN;
#else
  (*result)->type = TYPE_ANY;
#endif
  sub_expr = new_node_no_line();
  sub_expr->type = 0;
  sub_expr->v.expr = expr;
  sub_expr->r.expr = (*result)->r.expr;
  (*result)->r.expr = sub_expr;
  *result = check_refs(num_refs - saved_refs, opt_arg_list, *result);
  num_refs = saved_refs;
}

void rule_comma_expr(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  CREATE_TWO_VALUES(*result, expr2->type, pop_value(expr1), expr2);
}

void rule_expr_nullish(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  CREATE_NULLISH(*result, expr1, expr2);
}

void rule_expr_lor(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  CREATE_LAND_LOR(*result, F_LOR, expr1, expr2);
  if (IS_NODE(expr1, NODE_LAND_LOR, F_LOR))
    expr1->kind = NODE_BRANCH_LINK;
}

void rule_expr_land(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  CREATE_LAND_LOR(*result, F_LAND, expr1, expr2);
  if (IS_NODE(expr1, NODE_LAND_LOR, F_LAND))
    expr1->kind = NODE_BRANCH_LINK;
}

void rule_expr_or(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  int t1 = expr1->type, t3 = expr2->type;
  if (is_boolean(expr1) && is_boolean(expr2))
    yywarn("bitwise operation on boolean values.");
  if ((t1 & TYPE_MOD_ARRAY) || (t3 & TYPE_MOD_ARRAY)) {
    if (t1 != t3) {
      if ((t1 != TYPE_ANY) && (t3 != TYPE_ANY) && !(t1 & t3 & TYPE_MOD_ARRAY)) {
        char buf[256];
        char *end = EndOf(buf);
        char *p;
        p = strput(buf, end, "Incompatible types for | ");
        p = get_two_types(p, end, t1, t3);
        p = strput(p, end, ".");
        yyerror(buf);
      }
      t1 = TYPE_ANY | TYPE_MOD_ARRAY;
    }
    CREATE_BINARY_OP(*result, F_OR, t1, expr1, expr2);
  }
  else *result = binary_int_op(expr1, expr2, F_OR, "|");
}

void rule_expr_xor(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  *result = binary_int_op(expr1, expr2, F_XOR, "^");
}

void rule_expr_and(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  int t1 = expr1->type, t3 = expr2->type;
  if (is_boolean(expr1) && is_boolean(expr2))
    yywarn("bitwise operation on boolean values.");
  if ((t1 & TYPE_MOD_ARRAY) || (t3 & TYPE_MOD_ARRAY)) {
    if (t1 != t3) {
      if ((t1 != TYPE_ANY) && (t3 != TYPE_ANY) && !(t1 & t3 & TYPE_MOD_ARRAY)) {
        char buf[256];
        char *end = EndOf(buf);
        char *p;
        p = strput(buf, end, "Incompatible types for & ");
        p = get_two_types(p, end, t1, t3);
        p = strput(p, end, ".");
        yyerror(buf);
      }
      t1 = TYPE_ANY | TYPE_MOD_ARRAY;
    }
    CREATE_BINARY_OP(*result, F_AND, t1, expr1, expr2);
  } else *result = binary_int_op(expr1, expr2, F_AND, "&");
}

// L_EQ_NE carries the opcode (L_ORDER idiom); the two legs keep their
// separate bodies -- '==' has a compare-against-zero strength reduction
// '!=' doesn't.
void rule_expr_eq_ne(struct parse_node_t **result, LPC_INT op, struct parse_node_t *expr1,
                     struct parse_node_t *expr2) {
  if (op == F_EQ) {
    rule_expr_eq(result, expr1, expr2);
  } else {
    rule_expr_ne(result, expr1, expr2);
  }
}

void rule_expr_eq(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  if (exact_types && !compatible_types2(expr1->type, expr2->type)){
    char buf[256];
    char *end = EndOf(buf);
    char *p;
    p = strput(buf, end, "== always false because of incompatible types ");
    p = get_two_types(p, end, expr1->type, expr2->type);
    p = strput(p, end, ".");
    yyerror(buf);
  }
  if (IS_NODE(expr1, NODE_NUMBER, 0)) {
    CREATE_UNARY_OP(*result, F_NOT, TYPE_NUMBER, expr2);
  } else if (IS_NODE(expr2, NODE_NUMBER, 0)) {
    CREATE_UNARY_OP(*result, F_NOT, TYPE_NUMBER, expr1);
  } else {
    CREATE_BINARY_OP(*result, F_EQ, TYPE_NUMBER, expr1, expr2);
  }
}

void rule_expr_ne(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  if (exact_types && !compatible_types2(expr1->type, expr2->type)){
    char buf[256];
    char *end = EndOf(buf);
    char *p;
    p = strput(buf, end, "!= always true because of incompatible types ");
    p = get_two_types(p, end, expr1->type, expr2->type);
    p = strput(p, end, ".");
    yyerror(buf);
  }
  CREATE_BINARY_OP(*result, F_NE, TYPE_NUMBER, expr1, expr2);
}

void rule_expr_order(struct parse_node_t **result, struct parse_node_t *expr1, LPC_INT op, struct parse_node_t *expr2) {
  if (exact_types) {
    int t1 = expr1->type;
    int t3 = expr2->type;

    if (!COMP_TYPE(t1, TYPE_NUMBER) && !COMP_TYPE(t1, TYPE_STRING)) {
      char buf[256];
      char *end = EndOf(buf);
      char *p;
      p = strput(buf, end, "Bad left argument to '");
      p = strput(p, end, query_instr_name(op));
      p = strput(p, end, "' : \"");
      p = get_type_name(p, end, t1);
      p = strput(p, end, "\"");
      yyerror(buf);
    } else if (!COMP_TYPE(t3, TYPE_NUMBER) && !COMP_TYPE(t3, TYPE_STRING)) {
      char buf[256];
      char *end = EndOf(buf);
      char *p;
      p = strput(buf, end, "Bad right argument to '");
      p = strput(p, end, query_instr_name(op));
      p = strput(p, end, "' : \"");
      p = get_type_name(p, end, t3);
      p = strput(p, end, "\"");
      yyerror(buf);
    } else if (!compatible_types2(t1,t3)) {
      char buf[256];
      char *end = EndOf(buf);
      char *p;
      p = strput(buf, end, "Arguments to ");
      p = strput(p, end, query_instr_name(op));
      p = strput(p, end, " do not have compatible types : ");
      p = get_two_types(p, end, t1, t3);
      yyerror(buf);
    }
  }
  CREATE_BINARY_OP(*result, op, TYPE_NUMBER, expr1, expr2);
}

void rule_expr_lt(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  if (exact_types) {
    int t1 = expr1->type, t3 = expr2->type;

    if (!COMP_TYPE(t1, TYPE_NUMBER) && !COMP_TYPE(t1, TYPE_STRING)) {
      char buf[256];
      char *end = EndOf(buf);
      char *p;
      p = strput(buf, end, "Bad left argument to '<' : \"");
      p = get_type_name(p, end, t1);
      p = strput(p, end, "\"");
      yyerror(buf);
    } else if (!COMP_TYPE(t3, TYPE_NUMBER) && !COMP_TYPE(t3, TYPE_STRING)) {
      char buf[200];
      char *end = EndOf(buf);
      char *p;
      p = strput(buf, end, "Bad right argument to '<' : \"");
      p = get_type_name(p, end, t3);
      p = strput(p, end, "\"");
      yyerror(buf);
    } else if (!compatible_types2(t1,t3)) {
      char buf[256];
      char *end = EndOf(buf);
      char *p;
      p = strput(buf, end, "Arguments to < do not have compatible types : ");
      p = get_two_types(p, end, t1, t3);
      yyerror(buf);
    }
  }
  CREATE_BINARY_OP(*result, F_LT, TYPE_NUMBER, expr1, expr2);
}

void rule_expr_shift(struct parse_node_t **result, LPC_INT op, struct parse_node_t *expr1,
                     struct parse_node_t *expr2) {
  *result = binary_int_op(expr1, expr2, op, op == F_LSH ? "<<" : ">>");
}

void rule_expr_add(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  int result_type;

  if (exact_types) {
    int t1 = expr1->type, t3 = expr2->type;

    if (t1 == t3){
#ifdef CAST_CALL_OTHERS
      if (t1 == TYPE_UNKNOWN){
        yyerror("Bad arguments to '+' (unknown vs unknown)");
        result_type = TYPE_ANY;
      } else
#endif
        result_type = t1;
    }
    else if (t1 == TYPE_ANY) {
      if (t3 == TYPE_FUNCTION) {
        yyerror("Bad right argument to '+' (function)");
        result_type = TYPE_ANY;
      } else result_type = t3;
    } else if (t3 == TYPE_ANY) {
      if (t1 == TYPE_FUNCTION) {
        yyerror("Bad left argument to '+' (function)");
        result_type = TYPE_ANY;
      } else result_type = t1;
    } else {
      switch(t1) {
        case TYPE_OBJECT:
          if(t3 == TYPE_STRING){
            result_type = TYPE_STRING;
          } else goto add_error;
          break;
        case TYPE_STRING:
          {
            if (t3 == TYPE_REAL || t3 == TYPE_NUMBER || t3 == TYPE_OBJECT){
              result_type = TYPE_STRING;
            } else goto add_error;
            break;
          }
        case TYPE_NUMBER:
          {
            if (t3 == TYPE_REAL || t3 == TYPE_STRING)
              result_type = t3;
            else goto add_error;
            break;
          }
        case TYPE_REAL:
          {
            if (t3 == TYPE_NUMBER) result_type = TYPE_REAL;
            else if (t3 == TYPE_STRING) result_type = TYPE_STRING;
            else goto add_error;
            break;
          }
        default:
          {
            if (t1 & t3 & TYPE_MOD_ARRAY) {
              result_type = TYPE_ANY|TYPE_MOD_ARRAY;
              break;
            }
        add_error:
            {
              char buf[256];
              char *end = EndOf(buf);
              char *p;

              p = strput(buf, end, "Invalid argument types to '+' ");
              p = get_two_types(p, end, t1, t3);
              yyerror(buf);
              result_type = TYPE_ANY;
            }
          }
      }
    }
  } else
    result_type = TYPE_ANY;

  switch (expr1->kind) {
    case NODE_NUMBER:
      if (expr1->v.number == 0 &&
          (expr2->type == TYPE_NUMBER || expr2->type == TYPE_REAL)) {
        *result = expr2;
        break;
      }
      if (expr2->kind == NODE_NUMBER) {
        *result = expr1;
        expr1->v.number += expr2->v.number;
        break;
      }
      if (expr2->kind == NODE_REAL) {
        *result = expr2;
        expr2->v.real += expr1->v.number;
        break;
      }
      if (expr2->type != TYPE_STRING && expr2->type != TYPE_ANY)
        CREATE_BINARY_OP(*result, F_ADD, result_type, expr2, expr1);
      else
        CREATE_BINARY_OP(*result, F_ADD, result_type, expr1, expr2);
      break;
    case NODE_REAL:
      if (expr2->kind == NODE_NUMBER) {
        *result = expr1;
        expr1->v.real += expr2->v.number;
        break;
      }
      if (expr2->kind == NODE_REAL) {
        *result = expr1;
        expr1->v.real += expr2->v.real;
        break;
      }
      if (expr2->type != TYPE_STRING && expr2->type != TYPE_ANY)
        CREATE_BINARY_OP(*result, F_ADD, result_type, expr2, expr1);
      else
        CREATE_BINARY_OP(*result, F_ADD, result_type, expr1, expr2);
      break;
    case NODE_STRING:
      if (expr2->kind == NODE_STRING) {
        LPC_INT n1, n2;
        const char *s1, *s2;
        char *news;
        int l;

        n1 = expr1->v.number;
        n2 = expr2->v.number;
        s1 = PROG_STRING(n1);
        s2 = PROG_STRING(n2);
        news = (char *)DMALLOC( (l = strlen(s1))+strlen(s2)+1, TAG_COMPILER, "combine string" );
        strcpy(news, s1);
        strcat(news + l, s2);
        if (n1 > n2) {
          free_prog_string(n1); free_prog_string(n2);
        } else {
          free_prog_string(n2); free_prog_string(n1);
        }
        *result = expr1;
        (*result)->v.number = store_prog_string(news);
        FREE(news);
        break;
      }
      [[fallthrough]];
    default:
      if (IS_NODE(expr2, NODE_NUMBER, 0) &&
          (expr1->type == TYPE_NUMBER || expr1->type == TYPE_REAL)) {
        *result = expr1;
        break;
      }
      CREATE_BINARY_OP(*result, F_ADD, result_type, expr1, expr2);
      break;
  }
}

void rule_expr_sub(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  int result_type;

  if (exact_types) {
    int t1 = expr1->type, t3 = expr2->type;

    if (t1 == t3){
      switch(t1){
        case TYPE_ANY:
        case TYPE_NUMBER:
        case TYPE_REAL:
          result_type = t1;
          break;
        default:
          if (!(t1 & TYPE_MOD_ARRAY)){
            type_error("Bad argument number 1 to '-'", t1);
            result_type = TYPE_ANY;
          } else result_type = t1;
      }
    } else if (t1 == TYPE_ANY){
      switch(t3){
        case TYPE_REAL:
        case TYPE_NUMBER:
          result_type = t3;
          break;
        default:
          if (!(t3 & TYPE_MOD_ARRAY)){
            type_error("Bad argument number 2 to '-'", t3);
            result_type = TYPE_ANY;
          } else result_type = t3;
      }
    } else if (t3 == TYPE_ANY){
      switch(t1){
        case TYPE_REAL:
        case TYPE_NUMBER:
          result_type = t1;
          break;
        default:
          if (!(t1 & TYPE_MOD_ARRAY)){
            type_error("Bad argument number 1 to '-'", t1);
            result_type = TYPE_ANY;
          } else result_type = t1;
      }
    } else if ((t1 == TYPE_REAL && t3 == TYPE_NUMBER) ||
        (t3 == TYPE_REAL && t1 == TYPE_NUMBER)){
      result_type = TYPE_REAL;
    } else if (t1 & t3 & TYPE_MOD_ARRAY){
      result_type = TYPE_MOD_ARRAY|TYPE_ANY;
    } else {
      char buf[256];
      char *end = EndOf(buf);
      char *p;

      p = strput(buf, end, "Invalid types to '-' ");
      p = get_two_types(p, end, t1, t3);
      yyerror(buf);
      result_type = TYPE_ANY;
    }
  } else result_type = TYPE_ANY;

  switch (expr1->kind) {
    case NODE_NUMBER:
      if (expr1->v.number == 0) {
        CREATE_UNARY_OP(*result, F_NEGATE, expr2->type, expr2);
      } else if (expr2->kind == NODE_NUMBER) {
        *result = expr1;
        expr1->v.number -= expr2->v.number;
      } else if (expr2->kind == NODE_REAL) {
        *result = expr2;
        expr2->v.real = expr1->v.number - expr2->v.real;
      } else {
        CREATE_BINARY_OP(*result, F_SUBTRACT, result_type, expr1, expr2);
      }
      break;
    case NODE_REAL:
      if (expr2->kind == NODE_NUMBER) {
        *result = expr1;
        expr1->v.real -= expr2->v.number;
      } else if (expr2->kind == NODE_REAL) {
        *result = expr1;
        expr1->v.real -= expr2->v.real;
      } else {
        CREATE_BINARY_OP(*result, F_SUBTRACT, result_type, expr1, expr2);
      }
      break;
    default:
      if (IS_NODE(expr2, NODE_NUMBER, 0)) {
        *result = expr1;
        return;
      }
      CREATE_BINARY_OP(*result, F_SUBTRACT, result_type, expr1, expr2);
  }
}

void rule_expr_mul(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  int result_type;

  if (exact_types){
    int t1 = expr1->type, t3 = expr2->type;

    if (t1 == t3){
      switch(t1){
        case TYPE_MAPPING:
        case TYPE_ANY:
        case TYPE_NUMBER:
        case TYPE_REAL:
          result_type = t1;
          break;
        default:
          type_error("Bad argument number 1 to '*'", t1);
          result_type = TYPE_ANY;
      }
    } else if (t1 == TYPE_ANY || t3 == TYPE_ANY){
      int t = (t1 == TYPE_ANY) ? t3 : t1;
      switch(t){
        case TYPE_NUMBER:
        case TYPE_REAL:
        case TYPE_MAPPING:
          result_type = t;
          break;
        default:
          type_error((t1 == TYPE_ANY) ?
              "Bad argument number 2 to '*'" :
              "Bad argument number 1 to '*'",
              t);
          result_type = TYPE_ANY;
      }
    } else if ((t1 == TYPE_NUMBER && t3 == TYPE_REAL) ||
        (t1 == TYPE_REAL && t3 == TYPE_NUMBER)){
      result_type = TYPE_REAL;
    } else {
      char buf[256];
      char *end = EndOf(buf);
      char *p;

      p = strput(buf, end, "Invalid types to '*' ");
      p = get_two_types(p, end, t1, t3);
      yyerror(buf);
      result_type = TYPE_ANY;
    }
  } else result_type = TYPE_ANY;

  switch (expr1->kind) {
    case NODE_NUMBER:
      if (expr2->kind == NODE_NUMBER) {
        *result = expr1;
        (*result)->v.number *= expr2->v.number;
        break;
      }
      if (expr2->kind == NODE_REAL) {
        *result = expr2;
        expr2->v.real *= expr1->v.number;
        break;
      }
      CREATE_BINARY_OP(*result, F_MULTIPLY, result_type, expr2, expr1);
      break;
    case NODE_REAL:
      if (expr2->kind == NODE_NUMBER) {
        *result = expr1;
        expr1->v.real *= expr2->v.number;
        break;
      }
      if (expr2->kind == NODE_REAL) {
        *result = expr1;
        expr1->v.real *= expr2->v.real;
        break;
      }
      CREATE_BINARY_OP(*result, F_MULTIPLY, result_type, expr2, expr1);
      break;
    default:
      CREATE_BINARY_OP(*result, F_MULTIPLY, result_type, expr1, expr2);
  }
}

void rule_expr_mod(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  *result = binary_int_op(expr1, expr2, F_MOD, "%");
}

void rule_expr_div(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  int result_type;

  if (exact_types){
    int t1 = expr1->type, t3 = expr2->type;

    if (t1 == t3){
      switch(t1){
        case TYPE_NUMBER:
        case TYPE_REAL:
        case TYPE_ANY:
          result_type = t1;
          break;
        default:
          type_error("Bad argument 1 to '/'", t1);
          result_type = TYPE_ANY;
      }
    } else if (t1 == TYPE_ANY || t3 == TYPE_ANY){
      int t = (t1 == TYPE_ANY) ? t3 : t1;
      if (t == TYPE_REAL || t == TYPE_NUMBER)
        result_type = t;
      else {
        type_error(t1 == TYPE_ANY ?
            "Bad argument 2 to '/'" :
            "Bad argument 1 to '/'", t);
        result_type = TYPE_ANY;
      }
    } else if ((t1 == TYPE_NUMBER && t3 == TYPE_REAL) ||
        (t1 == TYPE_REAL && t3 == TYPE_NUMBER)) {
      result_type = TYPE_REAL;
    } else {
      char buf[256];
      char *end = EndOf(buf);
      char *p;

      p = strput(buf, end, "Invalid types to '/' ");
      p = get_two_types(p, end, t1, t3);
      yyerror(buf);
      result_type = TYPE_ANY;
    }
  } else result_type = TYPE_ANY;

  switch (expr1->kind) {
    case NODE_NUMBER:
      if (expr2->kind == NODE_NUMBER) {
        if (expr2->v.number == 0) {
          yyerror("Divide by zero in constant");
          *result = expr1;
          break;
        }
        *result = expr1;
        expr1->v.number /= expr2->v.number;
        break;
      }
      if (expr2->kind == NODE_REAL) {
        if (expr2->v.real == 0.0) {
          yyerror("Divide by zero in constant");
          *result = expr1;
          break;
        }
        *result = expr2;
        expr2->v.real = (expr1->v.number / expr2->v.real);
        break;
      }
      CREATE_BINARY_OP(*result, F_DIVIDE, result_type, expr1, expr2);
      break;
    case NODE_REAL:
      if (expr2->kind == NODE_NUMBER) {
        if (expr2->v.number == 0) {
          yyerror("Divide by zero in constant");
          *result = expr1;
          break;
        }
        *result = expr1;
        expr1->v.real /= expr2->v.number;
        break;
      }
      if (expr2->kind == NODE_REAL) {
        if (expr2->v.real == 0.0) {
          yyerror("Divide by zero in constant");
          *result = expr1;
          break;
        }
        *result = expr1;
        expr1->v.real /= expr2->v.real;
        break;
      }
      CREATE_BINARY_OP(*result, F_DIVIDE, result_type, expr1, expr2);
      break;
    default:
      CREATE_BINARY_OP(*result, F_DIVIDE, result_type, expr1, expr2);
  }
}

void rule_expr_cast(struct parse_node_t **result, LPC_INT type, struct parse_node_t *expr) {
  *result = expr;
  (*result)->type = type;

  if (exact_types &&
      expr->type != type &&
      expr->type != TYPE_ANY &&
      expr->type != TYPE_UNKNOWN &&
      type != TYPE_VOID) {
    char buf[256];
    char *end = EndOf(buf);
    char *p;

    p = strput(buf, end, "Cannot cast ");
    p = get_type_name(p, end, expr->type);
    p = strput(p, end, "to ");
    p = get_type_name(p, end, type);
    yyerror(buf);
  }
}

// ++/-- share one worker (L_INC_DEC carries '+' or '-'; pre/post comes
// from the production position). CREATE_UNARY_OP already stores the
// opcode in v.number.
static void expr_incdec(struct parse_node_t **result, int opcode, const char *badmsg,
                        struct parse_node_t *expr) {
  CREATE_UNARY_OP(*result, opcode, 0, expr);
  if (exact_types) {
    switch (expr->type) {
      case TYPE_NUMBER:
      case TYPE_ANY:
      case TYPE_REAL:
        (*result)->type = expr->type;
        break;
      default:
        (*result)->type = TYPE_ANY;
        type_error(badmsg, expr->type);
    }
  } else {
    (*result)->type = TYPE_ANY;
  }
}

void rule_expr_not(struct parse_node_t **result, struct parse_node_t *expr) {
  if (expr->kind == NODE_NUMBER) {
    *result = expr;
    (*result)->v.number = !((*result)->v.number);
  } else {
    CREATE_UNARY_OP(*result, F_NOT, TYPE_NUMBER, expr);
  }
}

void rule_expr_compl(struct parse_node_t **result, struct parse_node_t *expr) {
  if (exact_types && !IS_TYPE(expr->type, TYPE_NUMBER))
    type_error("Bad argument to ~", expr->type);
  if (expr->kind == NODE_NUMBER) {
    *result = expr;
    (*result)->v.number = ~(*result)->v.number;
  } else {
    CREATE_UNARY_OP(*result, F_COMPL, TYPE_NUMBER, expr);
  }
}
void rule_expr_neg(struct parse_node_t **result, struct parse_node_t *expr) {
  int result_type;
  if (exact_types){
    int t = expr->type;
    if (!COMP_TYPE(t, TYPE_NUMBER)){
      type_error("Bad argument to unary '-'", t);
      result_type = TYPE_ANY;
    } else result_type = t;
  } else result_type = TYPE_ANY;

  switch (expr->kind) {
    case NODE_NUMBER:
      *result = expr;
      (*result)->v.number = -(*result)->v.number;
      break;
    case NODE_REAL:
      *result = expr;
      (*result)->v.real = -(*result)->v.real;
      break;
    default:
      CREATE_UNARY_OP(*result, F_NEGATE, result_type, expr);
  }
}

void rule_expr_pre_incdec(struct parse_node_t **result, LPC_INT op, struct parse_node_t *expr) {
  if (op == '+') {
    expr_incdec(result, F_PRE_INC, "Bad argument 1 to ++x", expr);
  } else {
    expr_incdec(result, F_PRE_DEC, "Bad argument 1 to --x", expr);
  }
}

void rule_expr_post_incdec(struct parse_node_t **result, LPC_INT op, struct parse_node_t *expr) {
  if (op == '+') {
    expr_incdec(result, F_POST_INC, "Bad argument 1 to x++", expr);
  } else {
    expr_incdec(result, F_POST_DEC, "Bad argument 1 to x--", expr);
  }
}

void rule_opt_arg_list_empty(struct parse_node_t **result) {
  CREATE_EXPR_LIST(*result, 0);
}

void rule_opt_arg_list(struct parse_node_t **result, struct parse_node_t *expr) {
  CREATE_EXPR_LIST(*result, expr);
}

void rule_spread_expr_normal(struct parse_node_t **result, struct parse_node_t *expr) {
  CREATE_EXPR_NODE(*result, expr, 0);
}

void rule_spread_expr_dots(struct parse_node_t **result, struct parse_node_t *expr) {
  CREATE_EXPR_NODE(*result, expr, 1);
}

void rule_arg_list_single(struct parse_node_t **result, struct parse_node_t *expr) {
  expr->kind = 1;
  *result = expr;
}

void rule_arg_list_multi(struct parse_node_t **result, struct parse_node_t *list, struct parse_node_t *expr) {
  expr->kind = 0;
  *result = list;
  (*result)->kind++;
  (*result)->l.expr->r.expr = expr;
  (*result)->l.expr = expr;
}

void rule_opt_pair_list_empty(struct parse_node_t **result) {
  CREATE_EXPR_LIST(*result, 0);
}

void rule_opt_pair_list(struct parse_node_t **result, struct parse_node_t *expr) {
  CREATE_EXPR_LIST(*result, expr);
}

void rule_pair_list_single(struct parse_node_t **result, struct parse_node_t *expr) {
  *result = new_node_no_line();
  (*result)->kind = 2;
  (*result)->v.expr = expr;
  (*result)->r.expr = 0;
  (*result)->type = 0;
  (*result)->l.expr = *result;
}

void rule_pair_list_multi(struct parse_node_t **result, struct parse_node_t *list, struct parse_node_t *expr) {
  parse_node_t *new_node_item = new_node_no_line();
  new_node_item->kind = 0;
  new_node_item->v.expr = expr;
  new_node_item->r.expr = 0;
  new_node_item->type = 0;

  list->l.expr->r.expr = new_node_item;
  list->l.expr = new_node_item;
  list->kind += 2;
  *result = list;
}

void rule_assoc_pair(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2) {
  CREATE_TWO_VALUES(*result, 0, expr1, expr2);
}
