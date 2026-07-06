#include "base/std.h"

#include "compiler/internal/compiler_utils.h"

#include <cstdio>
#include <fmt/format.h>

#include "compiler/internal/grammar_rules.h"
#include "grammar.autogen.h"
#include "efuns.autogen.h"
#include "lex.h"
#include "compiler.h"
#include "vm/vm.h"

#define OR_BUFFER | T_BUFFER

#define add_instr_name(w, x, y, z) int_add_instr_name(w, y, z)

static void int_add_instr_name(const char *name, int n, short t) {
  instrs[n].name = name;
  instrs[n].ret_type = t;
}

void init_instrs() {
  unsigned int i, n;

  // operators
  for (i = 0; i < EFUN_BASE; i++) {
    instrs[i].name = operator_names[i];
    instrs[i].ret_type = T_ANY;
  }

  for (i = 0; i < size_of_predefs; i++) {
    n = predefs[i].token;
    if (n & F_ALIAS_FLAG) {
      predefs[i].token ^= F_ALIAS_FLAG;
    } else {
      instrs[n].min_arg = predefs[i].min_args;
      instrs[n].max_arg = predefs[i].max_args;
      instrs[n].name = predefs[i].word;
      instrs[n].type[0] = predefs[i].arg_type1;
      instrs[n].type[1] = predefs[i].arg_type2;
      instrs[n].type[2] = predefs[i].arg_type3;
      instrs[n].type[3] = predefs[i].arg_type4;
      instrs[n].Default = predefs[i].Default;
      instrs[n].ret_type = predefs[i].ret_type;
      instrs[n].arg_index = predefs[i].arg_index;
    }
  }

  add_instr_name("<", "c_lt();\n", F_LT, T_NUMBER);
  add_instr_name(">", "c_gt();\n", F_GT, T_NUMBER);
  add_instr_name("<=", "c_le();\n", F_LE, T_NUMBER);
  add_instr_name(">=", "c_ge();\n", F_GE, T_NUMBER);
  add_instr_name("==", "f_eq();\n", F_EQ, T_NUMBER);
  add_instr_name("+=", "c_add_eq(0);\n", F_ADD_EQ, T_ANY);
  add_instr_name("(void)+=", "c_add_eq(1);\n", F_VOID_ADD_EQ, T_NUMBER);
  add_instr_name("!", "c_not();\n", F_NOT, T_NUMBER);
  add_instr_name("&", "f_and();\n", F_AND, T_ARRAY | T_NUMBER);
  add_instr_name("&=", "f_and_eq();\n", F_AND_EQ, T_NUMBER);
  add_instr_name("index", "c_index();\n", F_INDEX, T_ANY);
  add_instr_name("member", "c_member(%i);\n", F_MEMBER, T_ANY);
  add_instr_name("new_empty_class", "c_new_class(%i, 0);\n", F_NEW_EMPTY_CLASS, T_ANY);
  add_instr_name("new_class", "c_new_class(%i, 1);\n", F_NEW_CLASS, T_ANY);
  add_instr_name("rindex", "c_rindex();\n", F_RINDEX, T_ANY);
  add_instr_name("loop_cond_local", "C_LOOP_COND_LV(%i, %i); if (lpc_int)\n", F_LOOP_COND_LOCAL,
                 -1);
  add_instr_name("loop_cond_number", "C_LOOP_COND_NUM(%i, %i); if (lpc_int)\n", F_LOOP_COND_NUMBER,
                 -1);
  add_instr_name("loop_incr", "C_LOOP_INCR(%i);\n", F_LOOP_INCR, -1);
  add_instr_name("foreach", 0, F_FOREACH, -1);
  add_instr_name("exit_foreach", "c_exit_foreach();\n", F_EXIT_FOREACH, -1);
  add_instr_name("expand_varargs", 0, F_EXPAND_VARARGS, -1);
  add_instr_name("next_foreach", "c_next_foreach();\n", F_NEXT_FOREACH, -1);
  add_instr_name("member_lvalue", "c_member_lvalue(%i);\n", F_MEMBER_LVALUE, T_LVALUE);
  add_instr_name("index_lvalue", "push_indexed_lvalue(0);\n", F_INDEX_LVALUE,
                 T_LVALUE | T_LVALUE_BYTE);
  add_instr_name("rindex_lvalue", "push_indexed_lvalue(1);\n", F_RINDEX_LVALUE,
                 T_LVALUE | T_LVALUE_BYTE);
  add_instr_name("nn_range_lvalue", "push_lvalue_range(0x00);\n", F_NN_RANGE_LVALUE,
                 T_LVALUE_RANGE);
  add_instr_name("nr_range_lvalue", "push_lvalue_range(0x01);\n", F_NR_RANGE_LVALUE,
                 T_LVALUE_RANGE);
  add_instr_name("rr_range_lvalue", "push_lvalue_range(0x11);\n", F_RR_RANGE_LVALUE,
                 T_LVALUE_RANGE);
  add_instr_name("rn_range_lvalue", "push_lvalue_range(0x10);\n", F_RN_RANGE_LVALUE,
                 T_LVALUE_RANGE);
  add_instr_name("nn_range", "f_range(0x00);\n", F_NN_RANGE, T_ARRAY | T_STRING OR_BUFFER);
  add_instr_name("rr_range", "f_range(0x11);\n", F_RR_RANGE, T_ARRAY | T_STRING OR_BUFFER);
  add_instr_name("nr_range", "f_range(0x01);\n", F_NR_RANGE, T_ARRAY | T_STRING OR_BUFFER);
  add_instr_name("rn_range", "f_range(0x10);\n", F_RN_RANGE, T_ARRAY | T_STRING OR_BUFFER);
  add_instr_name("re_range", "f_extract_range(1);\n", F_RE_RANGE, T_ARRAY | T_STRING OR_BUFFER);
  add_instr_name("ne_range", "f_extract_range(0);\n", F_NE_RANGE, T_ARRAY | T_STRING OR_BUFFER);
  add_instr_name("global", "C_GLOBAL(%i);\n", F_GLOBAL, T_ANY);
  add_instr_name("local", "C_LOCAL(%i);\n", F_LOCAL, T_ANY);
  add_instr_name("make_ref", "c_make_ref(%i);\n", F_MAKE_REF, T_REF);
  add_instr_name("kill_refs", "c_kill_refs(%i);\n", F_KILL_REFS, T_ANY);
  add_instr_name("ref", "C_REF(%i);\n", F_REF, T_ANY);
  add_instr_name("ref_lvalue", "C_REF_LVALUE(%i);\n", F_REF_LVALUE, T_LVALUE);
  add_instr_name("transfer_local", "C_TRANSFER_LOCAL(%i);\n", F_TRANSFER_LOCAL, T_ANY);
  add_instr_name("number", 0, F_NUMBER, T_NUMBER);
  add_instr_name("real", 0, F_REAL, T_REAL);
  add_instr_name("local_lvalue", "C_LVALUE(fp + %i);\n", F_LOCAL_LVALUE, T_LVALUE);
  add_instr_name("while_dec", "C_WHILE_DEC(%i); if (lpc_int)\n", F_WHILE_DEC, -1);
  add_instr_name("const1", "push_number(1);\n", F_CONST1, T_NUMBER);
  add_instr_name("subtract", "c_subtract();\n", F_SUBTRACT, T_NUMBER | T_REAL | T_ARRAY);
  add_instr_name("(void)assign", "c_void_assign();\n", F_VOID_ASSIGN, T_NUMBER);
  add_instr_name("(void)assign_local", "c_void_assign_local(fp + %i);\n", F_VOID_ASSIGN_LOCAL,
                 T_NUMBER);
  add_instr_name("assign", "c_assign();\n", F_ASSIGN, T_ANY);
  add_instr_name("branch", 0, F_BRANCH, -1);
  add_instr_name("bbranch", 0, F_BBRANCH, -1);
  add_instr_name("byte", 0, F_BYTE, T_NUMBER);
  add_instr_name("-byte", 0, F_NBYTE, T_NUMBER);
  add_instr_name("branch_ne", 0, F_BRANCH_NE, -1);
  add_instr_name("branch_ge", 0, F_BRANCH_GE, -1);
  add_instr_name("branch_le", 0, F_BRANCH_LE, -1);
  add_instr_name("branch_eq", 0, F_BRANCH_EQ, -1);
  add_instr_name("bbranch_lt", 0, F_BBRANCH_LT, -1);
  add_instr_name("bbranch_when_zero", 0, F_BBRANCH_WHEN_ZERO, -1);
  add_instr_name("bbranch_when_non_zero", 0, F_BBRANCH_WHEN_NON_ZERO, -1);
  add_instr_name("branch_when_zero", 0, F_BRANCH_WHEN_ZERO, -1);
  add_instr_name("branch_when_non_zero", 0, F_BRANCH_WHEN_NON_ZERO, -1);
  add_instr_name("pop", "pop_stack();\n", F_POP_VALUE, -1);
  add_instr_name("const0", "push_number(0);\n", F_CONST0, T_NUMBER);
#ifdef F_JUMP_WHEN_ZERO
  add_instr_name("jump_when_zero", F_JUMP_WHEN_ZERO, -1);
  add_instr_name("jump_when_non_zero", F_JUMP_WHEN_NON_ZERO, -1);
#endif
#ifdef F_LOR
  add_instr_name("||", 0, F_LOR, -1);
  add_instr_name("&&", 0, F_LAND, -1);
#endif
#ifdef F_LOR_EQ
  add_instr_name("||=", 0, F_LOR_EQ, -1);
#endif
#ifdef F_LAND_EQ
  add_instr_name("&&=", 0, F_LAND_EQ, -1);
#endif
#ifdef F_NULLISH
  add_instr_name("??", 0, F_NULLISH, -1);
#endif
#ifdef F_NULLISH_EQ
  add_instr_name("??=", 0, F_NULLISH_EQ, -1);
#endif
#ifdef F_ASSIGN_VALUE
  add_instr_name("assign_value", 0, F_ASSIGN_VALUE, -1);
#endif
  add_instr_name("-=", "f_sub_eq();\n", F_SUB_EQ, T_ANY);
#ifdef F_JUMP
  add_instr_name("jump", F_JUMP, -1);
#endif
  add_instr_name("return_zero", "c_return_zero();\nreturn;\n", F_RETURN_ZERO, -1);
  add_instr_name("return", "c_return();\nreturn;\n", F_RETURN, -1);
  add_instr_name("sscanf", "c_sscanf(%i);\n", F_SSCANF, T_NUMBER);
  add_instr_name("parse_command", "c_parse_command(%i);\n", F_PARSE_COMMAND, T_NUMBER);
  add_instr_name("string", 0, F_STRING, T_STRING);
  add_instr_name("short_string", 0, F_SHORT_STRING, T_STRING);
  add_instr_name("F_CALL_FUNCTION_BY_ADDRESS", "c_call(%i, %i);\n", F_CALL_FUNCTION_BY_ADDRESS, T_ANY);
  add_instr_name("call_inherited", "c_call_inherited(%i, %i, %i);\n", F_CALL_INHERITED, T_ANY);
  add_instr_name("aggregate_assoc", "C_AGGREGATE_ASSOC(%i);\n", F_AGGREGATE_ASSOC, T_MAPPING);
#ifdef DEBUG
  add_instr_name("break_point", "break_point();\n", F_BREAK_POINT, -1);
#endif
  add_instr_name("aggregate", "C_AGGREGATE(%i);\n", F_AGGREGATE, T_ARRAY);
  add_instr_name("(::)", 0, F_FUNCTION_CONSTRUCTOR, T_FUNCTION);
  /* sorry about this one */
  add_instr_name("simul_efun",
                 "call_simul_efun(%i, (lpc_int = %i + num_varargs, num_varargs "
                 "= 0, lpc_int));\n",
                 F_SIMUL_EFUN, T_ANY);
  add_instr_name("global_lvalue",
                 "C_LVALUE(&current_object->variables[variable_index_offset + %i]);\n",
                 F_GLOBAL_LVALUE, T_LVALUE);
  add_instr_name("|", "f_or();\n", F_OR, T_ARRAY | T_NUMBER);
  add_instr_name("<<", "f_lsh();\n", F_LSH, T_NUMBER);
  add_instr_name(">>", "f_rsh();\n", F_RSH, T_NUMBER);
  add_instr_name(">>=", "f_rsh_eq();\n", F_RSH_EQ, T_NUMBER);
  add_instr_name("<<=", "f_lsh_eq();\n", F_LSH_EQ, T_NUMBER);
  add_instr_name("^", "f_xor();\n", F_XOR, T_NUMBER);
  add_instr_name("^=", "f_xor_eq();\n", F_XOR_EQ, T_NUMBER);
  add_instr_name("|=", "f_or_eq();\n", F_OR_EQ, T_NUMBER);
  add_instr_name("+", "c_add();\n", F_ADD, T_ANY);
  add_instr_name("!=", "f_ne();\n", F_NE, T_NUMBER);
  add_instr_name("catch", 0, F_CATCH, T_ANY);
  add_instr_name("end_catch", 0, F_END_CATCH, -1);
  add_instr_name("-", "c_negate();\n", F_NEGATE, T_NUMBER | T_REAL);
  add_instr_name("~", "c_compl();\n", F_COMPL, T_NUMBER);
  add_instr_name("++x", "c_pre_inc();\n", F_PRE_INC, T_NUMBER | T_REAL);
  add_instr_name("--x", "c_pre_dec();\n", F_PRE_DEC, T_NUMBER | T_REAL);
  add_instr_name("*", "c_multiply();\n", F_MULTIPLY, T_REAL | T_NUMBER | T_MAPPING);
  add_instr_name("*=", "f_mult_eq();\n", F_MULT_EQ, T_REAL | T_NUMBER | T_MAPPING);
  add_instr_name("/", "c_divide();\n", F_DIVIDE, T_REAL | T_NUMBER);
  add_instr_name("/=", "f_div_eq();\n", F_DIV_EQ, T_NUMBER | T_REAL);
  add_instr_name("%", "c_mod();\n", F_MOD, T_NUMBER);
  add_instr_name("%=", "f_mod_eq();\n", F_MOD_EQ, T_NUMBER);
  add_instr_name("inc(x)", "c_inc();\n", F_INC, -1);
  add_instr_name("dec(x)", "c_dec();\n", F_DEC, -1);
  add_instr_name("x++", "c_post_inc();\n", F_POST_INC, T_NUMBER | T_REAL);
  add_instr_name("x--", "c_post_dec();\n", F_POST_DEC, T_NUMBER | T_REAL);
  add_instr_name("switch", 0, F_SWITCH, -1);
  add_instr_name("time_expression", 0, F_TIME_EXPRESSION, -1);
  add_instr_name("end_time_expression", 0, F_END_TIME_EXPRESSION, T_NUMBER);
}

void smart_log(const char *error_file, int line, const char *what, int flag) {
  auto logs = prepare_logs(error_file, line, what, flag, pragmas & PRAGMA_ERROR_CONTEXT);
  for (auto &log : logs) {
    debug_message("%s", log.c_str());
  }

  auto res = fmt::to_string(fmt::join(logs, ""));
  push_malloced_string(add_slash(error_file));
  copy_and_push_string(res.c_str());
  safe_apply_master_ob(APPLY_LOG_ERROR, 2);
} /* smart_log() */
