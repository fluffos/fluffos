#pragma once

#include "base/std.h"

#include "vm/vm.h"
#include "vm/internal/base/machine.h"
#include "compiler/internal/compiler.h"
#include "compiler/internal/lex.h"
#include "compiler/internal/scratchpad.h"
#include "compiler/internal/generate.h"

// Structs used by value in rule helpers — must match grammar.y %union fields.
typedef struct {
  struct parse_node_t *node;
  int num;
} decl_t;

typedef struct {
  int num_local;
  int max_num_locals;
  int context;
  int save_current_type;
  int save_exact_types;
} func_block_t;

// ============================================================================
// grammar_rules.cc
// ============================================================================
void rule_program(parse_node_t *program_node);
bool rule_inheritence(parse_node_t **result_node, int type_mod, const ScratchString *inherit_file_name);
LPC_INT rule_func_type(LPC_INT type, LPC_INT optional_star, const ScratchString *identifier);
// Registers the prototype under a SHARED string (ref held by the function
// table) built from the arena `identifier`; writes it to *shared_name_out
// (the value-stack slot's shared_string member). See grammar.y `function`.
LPC_INT rule_func_proto(LPC_INT type, LPC_INT optional_star, const ScratchString *identifier,
                        const char **shared_name_out, argument_t argument);
// `identifier` is the SHARED string rule_func_proto registered (the
// function table holds its ref) -- see the union's shared_string member.
void rule_func(parse_node_t **function, LPC_INT type, LPC_INT optional_star, const char *identifier,
               argument_t argument, LPC_INT *func_types, parse_node_t **block_or_semi);
struct ident_hash_elem_t *rule_define_class(LPC_INT *classname_idx_out, const ScratchString *class_name);
void rule_define_class_members(struct ident_hash_elem_t *class_ihe, LPC_INT classname_idx);

// Helpers that eliminate direct macro / global-variable access from grammar.y.
// These are all thin wrappers over one-liners that need compiler.h internals.
LPC_INT rule_loop_open();
LPC_INT rule_special_context_open();
LPC_INT rule_block_open();
void rule_number(parse_node_t **result, LPC_INT val);
void rule_real(parse_node_t **result, LPC_FLOAT val);
void rule_primary_expr_parameter(parse_node_t **result, LPC_INT n);
void rule_program_append(parse_node_t **result, parse_node_t *prog, parse_node_t *def);
void rule_tree_block(parse_node_t **result, parse_node_t *block_node);
void rule_tree_expr(parse_node_t **result, parse_node_t *expr);
void rule_opt_semicolon();
ScratchString *rule_string_literal_concat(ScratchString *s1, ScratchString *s2);

// ============================================================================
// grammar_rules_types.cc
// ============================================================================
ScratchString *rule_identifier_defined_name(ident_hash_elem_t *ihe);
parse_node_t *rule_modifier_change(LPC_INT modifiers);
void rule_member_name(LPC_INT star_modifier, const ScratchString *identifier);
void rule_member_list_set_type(LPC_INT basic_type);
parse_node_t *rule_default_arg_value(parse_node_t *expr);
LPC_INT rule_type_modifier_list(LPC_INT modifier, LPC_INT list);
LPC_INT rule_type(LPC_INT modifiers, LPC_INT basic_type);
LPC_INT rule_atomic_type_class(ident_hash_elem_t *ihe);
LPC_INT rule_atomic_type_class_identifier(const ScratchString *identifier);
LPC_INT rule_param_decl_typed(LPC_INT type_star);
LPC_INT rule_param_decl_typed_name(LPC_INT type_star, const ScratchString *name, parse_node_t *default_val);
LPC_INT rule_param_decl_untyped_name(const ScratchString *name);
void rule_argument_varargs(argument_t *result, argument_t *arg_list);
void rule_argument_list_single(argument_t *result, LPC_INT new_arg_val);
void rule_argument_list_multi(argument_t *result, argument_t *list, LPC_INT new_arg_val);
LPC_INT rule_cast(LPC_INT basic_type, LPC_INT optional_star);
LPC_INT rule_opt_basic_type_empty();
void rule_single_new_local_def(LPC_INT *result, LPC_INT type, const ScratchString *name);

// ============================================================================
// grammar_rules_decls.cc
// ============================================================================
struct parse_node_t *rule_block_or_semi(struct parse_node_t *block_node);
void rule_def_global_var(LPC_INT type_val);
ScratchString *rule_new_local_name_redefine(ident_hash_elem_t *ihe);
void rule_new_name(LPC_INT star_modifier, const ScratchString *identifier);
void rule_new_name_with_init(LPC_INT star_modifier, const ScratchString *identifier, LPC_INT assign_val, parse_node_t *expr);
void rule_block(decl_t *result, parse_node_t *stmts_node, int entry_locals);
parse_node_t *rule_new_local_def(const ScratchString *name, LPC_INT type_star);
parse_node_t *rule_new_local_def_with_init(const ScratchString *name, LPC_INT type_star, LPC_INT assign_op, parse_node_t *expr);
parse_node_t *rule_single_new_local_def_with_init(LPC_INT local_num, LPC_INT assign_op, parse_node_t *expr);
void rule_local_declarations_set_type(LPC_INT basic_type);
void rule_local_declarations(decl_t *result, decl_t *decl1, decl_t *decl2);
void rule_block_statements_empty(decl_t *result);
void rule_block_statements_stmt(decl_t *result, parse_node_t *stmt, decl_t *stmts);
void rule_block_statements_decl(decl_t *result, decl_t *decl_stmt, decl_t *stmts);
void rule_block_statements_error(decl_t *result, decl_t *stmts);
void rule_local_declaration_statement_set_type(LPC_INT basic_type);
void rule_local_declaration_statement(decl_t *result, decl_t *decl_list);
void rule_local_name_list_single(decl_t *result, parse_node_t *node);
void rule_local_name_list_multi(decl_t *result, parse_node_t *node, decl_t *list);

// ============================================================================
// grammar_rules_loops.cc
// ============================================================================
void rule_while(parse_node_t **result, parse_node_t *expr, parse_node_t *stmt, LPC_INT saved_context);
void rule_do(parse_node_t **result, parse_node_t *stmt, parse_node_t *expr, LPC_INT saved_context);
void rule_cond(parse_node_t **result, parse_node_t *expr, parse_node_t *then_branch, parse_node_t *else_branch);
void rule_foreach_var_defined(decl_t *result, ident_hash_elem_t *ihe);
void rule_foreach_var_new_local(decl_t *result, LPC_INT local_num);
void rule_foreach_var_identifier(decl_t *result, const ScratchString *identifier);
void rule_foreach_vars_single(decl_t *result, decl_t *var);
void rule_foreach_vars_double(decl_t *result, decl_t *var1, decl_t *var2);
LPC_INT rule_foreach_open();
void rule_foreach(decl_t *result, decl_t *vars, parse_node_t *expr, parse_node_t *stmt, LPC_INT saved_context);
void rule_for_init_expr(decl_t *result, parse_node_t *expr);
void rule_for_init_local(decl_t *result, parse_node_t *expr);
void rule_for(decl_t *result, decl_t *first, parse_node_t *cond, parse_node_t *incr, parse_node_t *stmt, LPC_INT saved_context);
parse_node_t *rule_statement_expr(parse_node_t *expr);
parse_node_t *rule_statement_compound_stmt(decl_t decl_val);
parse_node_t *rule_statement_break();
parse_node_t *rule_statement_continue();
void rule_return_void(parse_node_t **result);
void rule_return_expr(parse_node_t **result, parse_node_t *expr);

// ============================================================================
// grammar_rules_switch.cc
// ============================================================================
void rule_switch_pre(LPC_INT *saved_context, LPC_INT *saved_cases_size);
void rule_switch(parse_node_t **result, parse_node_t *expr, decl_t *local_decls, parse_node_t *case_node, parse_node_t *switch_block_node, LPC_INT saved_context, LPC_INT saved_cases_size);
void rule_case_single(parse_node_t **result, parse_node_t *label);
void rule_case_range(parse_node_t **result, parse_node_t *label1, parse_node_t *label2);
void rule_case_range_from(parse_node_t **result, parse_node_t *label1);
void rule_case_range_to(parse_node_t **result, parse_node_t *label2);
void rule_case_default(parse_node_t **result);
void rule_case_label_constant(parse_node_t **result, LPC_INT val);
void rule_case_label_string(parse_node_t **result, const ScratchString *str);
void rule_constant_or(LPC_INT *result, LPC_INT val1, LPC_INT val2);
void rule_constant_xor(LPC_INT *result, LPC_INT val1, LPC_INT val2);
void rule_constant_and(LPC_INT *result, LPC_INT val1, LPC_INT val2);
void rule_constant_eq_ne(LPC_INT *result, LPC_INT op, LPC_INT val1, LPC_INT val2);
void rule_constant_order(LPC_INT *result, LPC_INT val1, LPC_INT op, LPC_INT val2);
void rule_constant_lt(LPC_INT *result, LPC_INT val1, LPC_INT val2);
void rule_constant_shift(LPC_INT *result, LPC_INT op, LPC_INT val1, LPC_INT val2);
void rule_constant_add(LPC_INT *result, LPC_INT val1, LPC_INT val2);
void rule_constant_sub(LPC_INT *result, LPC_INT val1, LPC_INT val2);
void rule_constant_mul(LPC_INT *result, LPC_INT val1, LPC_INT val2);
void rule_constant_mod(LPC_INT *result, LPC_INT val1, LPC_INT val2);
void rule_constant_div(LPC_INT *result, LPC_INT val1, LPC_INT val2);
void rule_constant_neg(LPC_INT *result, LPC_INT val);
void rule_constant_not(LPC_INT *result, LPC_INT val);
void rule_constant_compl(LPC_INT *result, LPC_INT val);
void rule_switch_block_case(parse_node_t **result, parse_node_t *case_node, parse_node_t *block_node);
void rule_switch_block_stmt(parse_node_t **result, parse_node_t *stmt_node, parse_node_t *block_node);
void rule_switch_block_empty(parse_node_t **result);

// ============================================================================
// grammar_rules_exprs.cc
// ============================================================================
parse_node_t *rule_lvalue_list_empty();
parse_node_t *rule_lvalue_list(parse_node_t *lvalue, parse_node_t *list);
parse_node_t *rule_lvalue(parse_node_t *expr);
parse_node_t *rule_class_init(const ScratchString *identifier, parse_node_t *expr);
parse_node_t *rule_opt_class_init_empty();
parse_node_t *rule_opt_class_init(parse_node_t *list, parse_node_t *class_init);
ScratchString *rule_function_name_colon_colon(ScratchString *identifier);
ScratchString *rule_function_name_type(LPC_INT basic_type, ScratchString *identifier);
ScratchString *rule_function_name_obj(ScratchString *obj, ScratchString *identifier);
parse_node_t *rule_expr_or_block_block(decl_t decl_val);
parse_node_t *rule_expr_or_block_expr(parse_node_t *expr);
void rule_catch(parse_node_t **result, parse_node_t *expr_or_block, LPC_INT saved_context);
void rule_sscanf(parse_node_t **result, parse_node_t *expr1, parse_node_t *expr2, parse_node_t *lvalue_list);
void rule_parse_command(parse_node_t **result, parse_node_t *expr1, parse_node_t *expr2, parse_node_t *expr3, parse_node_t *lvalue_list);
void rule_time_expression(parse_node_t **result, parse_node_t *expr_or_block, LPC_INT saved_context);
void rule_string(parse_node_t **result, const ScratchString *str);
void rule_string_like_concat(parse_node_t **result, parse_node_t *left, parse_node_t *right);
void rule_template_literal(parse_node_t **result, const ScratchString *head, parse_node_t *expr, parse_node_t *rest);
void rule_template_parts_tail(parse_node_t **result, const ScratchString *tail);
void rule_template_parts_middle(parse_node_t **result, const ScratchString *mid, parse_node_t *expr, parse_node_t *rest);
LPC_INT rule_efun_override(const ScratchString *identifier);
LPC_INT rule_efun_override_new();
LPC_INT rule_functional_open(LPC_INT val);
// Resolve a defined name to the functional-reference encoding
// ((index << 8) | FP_KIND) the functional_1/_2 constructors take --
// the switch the lexer's L_NEW_FUNCTION_OPEN path used to run (9.2).
LPC_INT rule_functional_ref(struct ident_hash_elem_t *ihe);
void rule_expr_ref(parse_node_t **result, parse_node_t *lval);
void rule_expr_assign(parse_node_t **result, parse_node_t *lval, int opcode, parse_node_t *rval);
void rule_expr_assign_error(parse_node_t **result, parse_node_t *expr);
void rule_expr_ternary(parse_node_t **result, parse_node_t *cond, parse_node_t *val1, parse_node_t *val2);
void rule_primary_expr_cast(parse_node_t **result, LPC_INT type, parse_node_t *expr);
void rule_primary_expr_pre_inc(parse_node_t **result, parse_node_t *lval);
void rule_primary_expr_pre_dec(parse_node_t **result, parse_node_t *lval);
void rule_primary_expr_post_inc(parse_node_t **result, parse_node_t *lval);
void rule_primary_expr_post_dec(parse_node_t **result, parse_node_t *lval);
void rule_primary_expr_defined_name(parse_node_t **result, ident_hash_elem_t *ihe);
void rule_primary_expr_identifier(parse_node_t **result, const ScratchString *name);
function_context_t *rule_dollar_open();
void rule_primary_expr_dollar_expr(parse_node_t **result, function_context_t *saved_context, parse_node_t *expr);
void rule_primary_expr_member_arrow(parse_node_t **result, parse_node_t *expr, const ScratchString *identifier);
void rule_primary_expr_member_dot(parse_node_t **result, parse_node_t *expr, const ScratchString *identifier);
void rule_primary_expr_member_optional(parse_node_t **result, parse_node_t *expr, const ScratchString *identifier);
void rule_primary_expr_index_optional(parse_node_t **result, parse_node_t *expr, parse_node_t *idx);
void rule_primary_expr_range_nn(parse_node_t **result, parse_node_t *expr, parse_node_t *expr1, parse_node_t *expr2);
void rule_primary_expr_range_rn(parse_node_t **result, parse_node_t *expr, parse_node_t *expr1, parse_node_t *expr2);
void rule_primary_expr_range_rr(parse_node_t **result, parse_node_t *expr, parse_node_t *expr1, parse_node_t *expr2);
void rule_primary_expr_range_nr(parse_node_t **result, parse_node_t *expr, parse_node_t *expr1, parse_node_t *expr2);
void rule_primary_expr_range_ne(parse_node_t **result, parse_node_t *expr, parse_node_t *expr1);
void rule_primary_expr_range_re(parse_node_t **result, parse_node_t *expr, parse_node_t *expr1);
void rule_primary_expr_index(parse_node_t **result, parse_node_t *expr, parse_node_t *idx);
void rule_primary_expr_index_r(parse_node_t **result, parse_node_t *expr, parse_node_t *idx);
void rule_lambda_return_type(func_block_t *saved_block, LPC_INT type);
void rule_primary_expr_anon_func(parse_node_t **result, func_block_t *saved_block, argument_t *arg, decl_t *block);
void rule_primary_expr_functional_1(parse_node_t **result, LPC_INT val);
void rule_primary_expr_functional_2(parse_node_t **result, LPC_INT val, parse_node_t *opt_arg_list);
void rule_primary_expr_functional_3(parse_node_t **result, parse_node_t *expr);
void rule_primary_expr_mapping(parse_node_t **result, parse_node_t *opt_arg_list);
void rule_primary_expr_array(parse_node_t **result, parse_node_t *opt_arg_list);
void rule_call_open(LPC_INT *saved_context, LPC_INT *saved_refs);
void rule_function_call_efun(parse_node_t **result, LPC_INT efun_idx, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs);
void rule_function_call_new(parse_node_t **result, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs);
void rule_function_call_new_class(parse_node_t **result, ident_hash_elem_t *ihe, parse_node_t *class_init);
void rule_function_call_new_class_undef(parse_node_t **result, const ScratchString *name, parse_node_t *class_init);
void rule_function_call_defined_name(parse_node_t **result, ident_hash_elem_t *ihe, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs);
void rule_function_call_name(parse_node_t **result, const ScratchString *name, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs);
void rule_function_call_indexed(parse_node_t **result, parse_node_t *expr, parse_node_t *idx, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs);
void rule_function_call_arrow(parse_node_t **result, parse_node_t *expr, const ScratchString *identifier, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs);
void rule_function_call_star(parse_node_t **result, parse_node_t *expr, parse_node_t *opt_arg_list, LPC_INT saved_context, LPC_INT saved_refs);
void rule_comma_expr(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_nullish(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_lor(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_land(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_or(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_xor(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_and(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_eq_ne(struct parse_node_t **result, LPC_INT op, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_eq(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_ne(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_order(struct parse_node_t **result, struct parse_node_t *expr1, LPC_INT op, struct parse_node_t *expr2);
void rule_expr_lt(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_shift(struct parse_node_t **result, LPC_INT op, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_add(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_sub(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_mul(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_mod(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_div(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
void rule_expr_cast(struct parse_node_t **result, LPC_INT type, struct parse_node_t *expr);
void rule_expr_pre_incdec(struct parse_node_t **result, LPC_INT op, struct parse_node_t *expr);
void rule_expr_not(struct parse_node_t **result, struct parse_node_t *expr);
void rule_expr_compl(struct parse_node_t **result, struct parse_node_t *expr);
void rule_expr_neg(struct parse_node_t **result, struct parse_node_t *expr);
void rule_expr_post_incdec(struct parse_node_t **result, LPC_INT op, struct parse_node_t *expr);
void rule_opt_arg_list_empty(struct parse_node_t **result);
void rule_opt_arg_list(struct parse_node_t **result, struct parse_node_t *expr);
void rule_spread_expr_normal(struct parse_node_t **result, struct parse_node_t *expr);
void rule_spread_expr_dots(struct parse_node_t **result, struct parse_node_t *expr);
void rule_arg_list_single(struct parse_node_t **result, struct parse_node_t *expr);
void rule_arg_list_multi(struct parse_node_t **result, struct parse_node_t *list, struct parse_node_t *expr);
void rule_opt_pair_list_empty(struct parse_node_t **result);
void rule_opt_pair_list(struct parse_node_t **result, struct parse_node_t *expr);
void rule_pair_list_single(struct parse_node_t **result, struct parse_node_t *expr);
void rule_pair_list_multi(struct parse_node_t **result, struct parse_node_t *list, struct parse_node_t *expr);
void rule_assoc_pair(struct parse_node_t **result, struct parse_node_t *expr1, struct parse_node_t *expr2);
