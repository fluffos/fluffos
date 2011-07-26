/*
 * This file specifies the operators used by the interpreter.
 * Normally, these should not be commented out.
 */

operator pop_value, push, efun0, efun1, efun2, efun3, efunv;

operator short_int, number, real, byte, nbyte, string, short_string, const0, const1;

operator aggregate, aggregate_assoc;
#ifdef DEBUG
operator break_point;
#endif

/* these must be set up so that F_BRANCH is the last foward branch and
 * F_BRANCH_X + 3 == F_BBRANCH_X
 */
operator branch_ne, branch_ge, branch_le, branch_eq;
operator branch_when_zero, branch_when_non_zero, branch;
operator bbranch_when_zero, bbranch_when_non_zero, bbranch;

operator bbranch_lt;

operator foreach, next_foreach, exit_foreach;
operator loop_cond_local, loop_cond_number;
operator loop_incr;
operator while_dec;

operator lor, land;

operator catch, end_catch;
operator time_expression, end_time_expression;

operator switch;

operator call_function_by_address, call_inherited, return, return_zero;

/* eq must be first, gt must be last; c.f. is_boolean() */
operator eq, ne, le, lt, ge, gt;

operator inc, dec, pre_inc, post_inc, pre_dec, post_dec;

operator transfer_local;

operator make_ref, kill_refs;

/* lvalue eops must be the original eop + 1 */
operator local, local_lvalue;
operator ref, ref_lvalue;
operator global, global_lvalue;
operator member, member_lvalue;
operator index, index_lvalue;
operator rindex, rindex_lvalue;
operator nn_range, nn_range_lvalue, rn_range, rn_range_lvalue;
operator rr_range, rr_range_lvalue, nr_range, nr_range_lvalue;
operator ne_range, re_range;

/* these must all be together */
operator add_eq, sub_eq, and_eq, or_eq, xor_eq, lsh_eq, rsh_eq, mult_eq;
operator div_eq, mod_eq, assign;

operator void_add_eq, void_assign, void_assign_local;

operator add, subtract, multiply, divide, mod, and, or, xor, lsh, rsh;
operator not, negate, compl;

operator function_constructor;
operator simul_efun;

operator sscanf;
operator parse_command;

operator new_class, new_empty_class;
operator expand_varargs;
operator type_check;

