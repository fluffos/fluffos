/*
 * This file specifies the operators used by the interpreter.
 * Normally, these should not be commented out.
 */

operator dup, pop_value;

operator number, real, byte, nbyte, string, const0, const1;

operator index, range;
operator aggregate, aggregate_assoc;
operator break_point;

operator jump;
operator branch, branch_when_zero, branch_when_non_zero;
operator bbranch_when_zero, bbranch_when_non_zero;
#ifdef LPC_OPTIMIZE_LOOPS
operator loop_cond;
operator loop_incr;
operator while_dec;
#endif
operator lor, land;

operator catch, end_catch;
operator time_expression, end_time_expression;

operator switch, break, pop_break;

operator call_function_by_address, return;

operator eq, ne, le, lt, ge, gt;

operator inc, dec, pre_inc, post_inc, pre_dec, post_dec;

operator local_name, identifier;
operator push_identifier_lvalue, push_local_variable_lvalue;
operator push_indexed_lvalue;

operator add, void_add_eq, add_eq;
operator subtract, sub_eq;
operator multiply, mult_eq;
operator divide, div_eq;
operator mod, mod_eq;
operator not;
operator and, and_eq;
operator or, or_eq;
operator xor, xor_eq;
operator lsh, lsh_eq, rsh, rsh_eq;
operator negate, compl;

operator assign, void_assign;

operator function_constructor, this_function_constructor, function_split;

operator sscanf;
operator parse_command;

operator call_extra;
