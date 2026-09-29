#include "base/std.h"

#include "vm/vm.h"
#include "vm/internal/base/machine.h"
#include "compiler/internal/compiler.h"
#include "compiler/internal/lexer.h"
#include "base/internal/scratchpad.h"
#include "compiler/internal/generate.h"
#include "compiler/internal/grammar_rules.h"

#include <fmt/format.h>

extern int context;
extern int func_present;
extern int num_refs;

struct parse_node_t* rule_block_or_semi(struct parse_node_t* block_node) {
  if (!block_node) {
    CREATE_RETURN(block_node, 0);
  }
  return block_node;
}

/* A string or array-of-ints initializer promotes to a buffer via
 * to_buffer() in do_promotions() -- exempt from the type-mismatch check. */
static bool buffer_promotable(int type, int exprtype) {
  return type == TYPE_BUFFER && (exprtype == TYPE_STRING || (exprtype & TYPE_MOD_ARRAY));
}

void rule_def_global_var(LPC_INT type_val) {
  /* End of a global declaration: every initializer in it has been compiled
   * into the __INIT tree, so the names those initializers declared go out of
   * scope here.
   *
   * release_local_names(), NOT free_all_local_names(), and the difference is
   * the whole point. Every global initializer in a file is compiled into ONE
   * frame -- __INIT -- so its slot numbering has to keep climbing across
   * declarations, exactly as it does across sibling scopes inside a function
   * (which is why pop_n_locals() lowers current_number_of_locals but never
   * max_num_locals). Resetting max_num_locals here as well made successive
   * initializers ALIAS slot 0, and a local declared WITHOUT an initializer
   * emits no code at all -- rule_new_local_def() leaves it undefined until
   * first assigned, relying on the frame's one-time zero-fill -- so it read
   * the previous declaration's leftover value:
   *
   *   mixed g1 = catch { int a = 42; };
   *   mixed g2 = catch { int b; seen = b; };   // seen == 42, not 0
   *
   * across types too: a `class A a;` reading a live `class B` left by an
   * earlier initializer, with no diagnostic.
   *
   * max_num_locals is reset instead where a genuinely new frame begins, in
   * rule_func_type() for each function definition. That is what keeps the
   * next function's parameters at slots 0..n-1 -- without it, one local
   * declared in a global initializer's block shifted them all and
   * `int id(int a) { return a; }` silently returned 0. __INIT's own frame is
   * sized from the compile-wide high-water mark (compile_max_num_locals). */
  release_local_names(0);

  if (!(type_val & ~(DECL_MODS))) {
    /* A typeless declaration immediately after a class body is almost
       certainly the C-style combined form 'class Foo { ... } var;', which
       LPC does not support -- the variable would silently get unknown
       type. Diagnose it here instead of letting '->' fail later (#788). */
    if (g_compile.class_def_cooldown == 1) {
      yyerror(
          "Variable declaration cannot follow a class body directly; "
          "declare it separately with the class type: 'class Name varname;'");
      return;
    }
    if (pragmas & PRAGMA_STRICT_TYPES) {
      yyerror("Missing type for global variable declaration");
    }
  }
}

ScratchString* rule_new_local_name_redefine(ident_hash_elem_t* ihe) {
  if (ihe->dn.local_num != -1) {
    yyerror("Illegal to redeclare local name '%s'", ihe->name);
  }
  return scratch_new_string(ihe->name);
}

void rule_new_name(LPC_INT star_modifier, const ScratchString* identifier) {
  if (current_type & PACK_TYPE_MODS(FUNC_VARARGS)) {
    yyerror("Illegal to declare varargs variable.");
    current_type &= ~PACK_TYPE_MODS(FUNC_VARARGS);
  }
  if (current_type & PACK_TYPE_MODS(FUNC_ASYNC)) {
    yyerror("Illegal to declare async variable.");
    current_type &= ~PACK_TYPE_MODS(FUNC_ASYNC);
  }

  if (current_type & ~BASIC_TYPE_MASK) {
    current_type = PACKED_TYPE_MODS(current_type) | PACKED_TYPE_BASIC(current_type);
  }

  current_type |= global_modifiers;

  if (!(current_type & DECL_ACCESS)) current_type |= DECL_PUBLIC;

  if ((current_type & ~DECL_MODS) == TYPE_VOID)
    yyerror("Illegal to declare global variable of type void.");

  /* No synthesized initializer here even for `float`: a declared variable
   * with no initializer must start undefined (const0u, undefinedp() == 1).
   * Its declared type is enforced on first assignment instead -- '=' via
   * do_promotions() and op= via the compile-time RHS coercion in
   * rule_expr_assign(). */
  define_new_variable(identifier, current_type | star_modifier);
}

void rule_new_name_with_init(LPC_INT star_modifier, const ScratchString* identifier,
                             LPC_INT assign_val, parse_node_t* expr) {
  parse_node_t *expr_node, *newnode;
  int type;

  if (current_type & PACK_TYPE_MODS(FUNC_VARARGS)) {
    yyerror("Illegal to declare varargs variable.");
    current_type &= ~PACK_TYPE_MODS(FUNC_VARARGS);
  }
  if (current_type & PACK_TYPE_MODS(FUNC_ASYNC)) {
    yyerror("Illegal to declare async variable.");
    current_type &= ~PACK_TYPE_MODS(FUNC_ASYNC);
  }

  if (current_type & ~BASIC_TYPE_MASK) {
    current_type = PACKED_TYPE_MODS(current_type) | PACKED_TYPE_BASIC(current_type);
  }

  current_type |= global_modifiers;

  if (!(current_type & DECL_ACCESS)) current_type |= DECL_PUBLIC;

  if ((current_type & ~DECL_MODS) == TYPE_VOID)
    yyerror("Illegal to declare global variable of type void.");

  if (assign_val != F_ASSIGN) yyerror("Only '=' is legal in initializers.");

  if (current_type) {
    type = (current_type | star_modifier) & ~DECL_MODS;
    if ((current_type & ~DECL_MODS) == TYPE_VOID)
      yyerror("Illegal to declare global variable of type void.");
    if (!compatible_types(type, expr->type) && !buffer_promotable(type, expr->type)) {
      char buff[256];
      char* end = EndOf(buff);
      char* p;

      p = strput(buff, end, "Type mismatch ");
      p = get_two_types(p, end, type, expr->type);
      p = strput(p, end, " when initializing ");
      p = strput(p, end, identifier->c_str());
      yyerror("%s", buff);
    }
  } else
    type = 0;
  expr = do_promotions(expr, type);

  CREATE_BINARY_OP(expr_node, F_VOID_ASSIGN, 0, expr, 0);
  CREATE_OPCODE_1(expr_node->r.expr, F_GLOBAL_LVALUE, 0,
                  define_new_variable(identifier, current_type | star_modifier));
  newnode = comp_trees[TREE_INIT];
  CREATE_TWO_VALUES(comp_trees[TREE_INIT], 0, newnode, expr_node);
}

void rule_block(decl_t* result, parse_node_t* stmts_node, int entry_locals) {
  result->node = stmts_node;
  result->num = current_number_of_locals - entry_locals;
  if (result->num < 0) {
    /* Bison error recovery can discard a production that had opened a nested
     * locals scope before the rule that would have restored it ever runs --
     * `function (int x, ) { ... }`, a stray comma, leaves
     * rule_lambda_return_type()'s scope switch unwound by
     * rule_primary_expr_anon_func(). The enclosing block then closes with
     * fewer locals than it entered with, and the count it hands up is
     * NEGATIVE: a clean fatal("pop_n_locals called with num < 0") on Debug,
     * and a SIGSEGV on a build where DEBUG_CHECK is compiled out, from a
     * syntax error in one line of mudlib source.
     *
     * The count is only ever used to decide how many names to take back out
     * of scope, and the compile is already failing, so clamping is exactly
     * right: it turns the crash back into the diagnostic the user should
     * have got. (Deliberately here rather than inside pop_n_locals(), whose
     * DEBUG_CHECK stays a real invariant for callers that compute a count
     * some other way.) */
    result->num = 0;
  }
}

parse_node_t* rule_new_local_def(const ScratchString* name, LPC_INT type_star) {
  if (current_type & LOCAL_MOD_REF) {
    yyerror("Illegal to declare local variable as reference");
    current_type &= ~LOCAL_MOD_REF;
  }
  /* Like rule_new_name(): no synthesized `= 0.0` for a bare `float` local --
   * it must start undefined (push_undefineds/const0u) until first assigned. */
  add_local_name(name, current_type | type_star | LOCAL_MOD_UNUSED);
  return nullptr;
}

parse_node_t* rule_new_local_def_with_init(const ScratchString* name, LPC_INT type_star,
                                           LPC_INT assign_op, parse_node_t* expr) {
  int type = (current_type | type_star) & ~DECL_MODS;

  if (current_type & LOCAL_MOD_REF) {
    yyerror("Illegal to declare local variable as reference");
    current_type &= ~LOCAL_MOD_REF;
    type &= ~LOCAL_MOD_REF;
  }

  if (assign_op != F_ASSIGN) yyerror("Only '=' is allowed in initializers.");
  if (!compatible_types(expr->type, type) && !buffer_promotable(type, expr->type)) {
    char buff[256];
    char* end = EndOf(buff);
    char* p;

    p = strput(buff, end, "Type mismatch ");
    p = get_two_types(p, end, type, expr->type);
    p = strput(p, end, " when initializing ");
    p = strput(p, end, name->c_str());
    yyerror("%s", buff);
  }

  expr = do_promotions(expr, type);

  parse_node_t* res;
  CREATE_UNARY_OP_1(res, F_VOID_ASSIGN_LOCAL, 0, expr,
                    add_local_name(name, current_type | type_star | LOCAL_MOD_UNUSED));
  return res;
}

parse_node_t* rule_single_new_local_def_with_init(LPC_INT local_num, LPC_INT assign_op,
                                                  parse_node_t* expr) {
  int type = type_of_locals_ptr[local_num];

  if (type & LOCAL_MOD_REF) {
    yyerror("Illegal to declare local variable as reference");
    type_of_locals_ptr[local_num] &= ~LOCAL_MOD_REF;
  }
  type &= ~LOCAL_MODS;

  if (assign_op != F_ASSIGN) yyerror("Only '=' is allowed in initializers.");
  if (!compatible_types(expr->type, type) && !buffer_promotable(type, expr->type)) {
    char buff[256];
    char* end = EndOf(buff);
    char* p;

    p = strput(buff, end, "Type mismatch ");
    p = get_two_types(p, end, type, expr->type);
    p = strput(p, end, " when initializing.");
    yyerror("%s", buff);
  }

  expr = do_promotions(expr, type);

  parse_node_t* res;
  CREATE_BINARY_OP(res, F_ASSIGN, 0, expr, 0);
  CREATE_OPCODE_1(res->r.expr, F_LOCAL_LVALUE, 0, local_num);
  return res;
}

void rule_local_declarations_set_type(LPC_INT basic_type) {
  if (basic_type == TYPE_VOID) yyerror("Illegal to declare local variable of type void.");
  current_type = basic_type;
}

void rule_local_declarations(decl_t* result, decl_t* decl1, decl_t* decl2) {
  if (decl1->node && decl2->node) {
    CREATE_STATEMENTS(result->node, decl1->node, decl2->node);
  } else {
    result->node = (decl1->node ? decl1->node : decl2->node);
  }
  result->num = decl1->num + decl2->num;
}

void rule_block_statements_empty(decl_t* result) {
  result->node = 0;
  result->num = 0;
}

void rule_block_statements_stmt(decl_t* result, parse_node_t* stmt, decl_t* stmts) {
  if (stmt && stmts->node) {
    CREATE_STATEMENTS(result->node, stmt, stmts->node);
  } else {
    result->node = (stmt ? stmt : stmts->node);
  }
  result->num = stmts->num;
}

void rule_block_statements_decl(decl_t* result, decl_t* decl_stmt, decl_t* stmts) {
  if (decl_stmt->node && stmts->node) {
    CREATE_STATEMENTS(result->node, decl_stmt->node, stmts->node);
  } else {
    result->node = (decl_stmt->node ? decl_stmt->node : stmts->node);
  }
  result->num = decl_stmt->num + stmts->num;
}

void rule_block_statements_error(decl_t* result, decl_t* stmts) {
  result->node = stmts->node;
  result->num = stmts->num;
}

void rule_local_declaration_statement_set_type(LPC_INT basic_type) {
  if (basic_type == TYPE_VOID) yyerror("Illegal to declare local variable of type void.");
  current_type = basic_type;
}

void rule_local_declaration_statement(decl_t* result, decl_t* decl_list) {
  result->node = decl_list->node;
  result->num = decl_list->num;
}

void rule_local_name_list_single(decl_t* result, parse_node_t* node) {
  result->node = node;
  result->num = 1;
}

void rule_local_name_list_multi(decl_t* result, parse_node_t* node, decl_t* list) {
  if (node && list->node) {
    CREATE_STATEMENTS(result->node, node, list->node);
  } else {
    result->node = (node ? node : list->node);
  }
  result->num = 1 + list->num;
}
