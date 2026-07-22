#include "base/std.h"

#include "generate.h"

#include <cstdio>
#include <vector>

#include <nlohmann/json.hpp>  // lpcc --ast --json envelope

#include "include/function.h"  // for F_SIMUL etc , FIXME
#include "efuns.autogen.h"
#include "keyword.h"                  // predefs[]: FP_EFUN name resolution
#include "vm/internal/base/number.h"  // for formatting lpc int

#include "compiler.h"  // for CURRENT_PROGRAM_SIZE
#include "lexer.h"     // for pragmas
#include "icode.h"     // for IS_NODE.

static parse_node_t* optimize(parse_node_t* /*expr*/);
static parse_node_t** last_local_refs = nullptr;
static int optimizer_num_locals;

/* Document optimizations here so we can make sure they don't interfere.
 *
 * Transfer of dying variables:
 * . If the last F_LOCAL was not in a loop, replace with transfer_local.
 * . Similarly, if an assign is done, change the last use to a transfer if safe
 * CAVEATS: we ignore while_dec, loop_cond, and loop_incr.  Justification is
 * that transfer_local doesn't clobber ints since it just sets type to T_NUMBER
 * This optimization also can't deal with code motion.  Since it assumes the
 * order optimized is the same as the order emitted.
 * It also can't detect that a variable dies along multiple branches, so:
 * if (y) { use(x); } else { use(x); } x = 1;
 * doesn't get optimized.
 */

static void optimize_expr_list(parse_node_t* expr) {
  if (!expr) {
    return;
  }
  do {
    expr->v.expr = optimize(expr->v.expr);
  } while ((expr = expr->r.expr));
}

static void optimize_lvalue_list(parse_node_t* expr) {
  while ((expr = expr->r.expr)) {
    expr->v.expr = optimize(expr->l.expr);
  }
}

#define OPT(x) x = optimize(x)
#define OPTIMIZER_IN_LOOP 1
#define OPTIMIZER_IN_COND 2 /* switch or if or ?: */
static int optimizer_state = 0;

namespace {
// Mirrors the guard in icode.cc's i_generate_node(): a left-nested chain
// of binary/unary/ternary operators builds a parse tree as deep as the
// input, with no bound from the grammar/parser. The NODE_TWO_VALUES case
// below walks its own chain (one node per top-level definition/statement)
// iteratively rather than recursing, so this cap bounds genuine
// expression nesting only -- never an object's definition or statement
// count (github.com/fluffos/fluffos/issues/1267).
//
// Unlike i_generate_node(), going over is not a compile error: skipping
// optimization of a subtree still leaves correct (just less tight)
// bytecode, so this only warns and gives up on that subtree.
int g_optimize_depth = 0;
constexpr int kMaxOptimizeDepth = 500;
}  // namespace

static parse_node_t* optimize(parse_node_t* expr) {
  if (!expr) {
    return nullptr;
  }
  if (++g_optimize_depth > kMaxOptimizeDepth) {
    --g_optimize_depth;
    yywarn("Expression nested too deeply to fully optimize.");
    return expr;
  }
  DEFER { --g_optimize_depth; };

  switch (expr->kind) {
    case NODE_TERNARY_OP:
      OPT(expr->l.expr);
      OPT(expr->r.expr->l.expr);
      OPT(expr->r.expr->r.expr);
      break;
    case NODE_BINARY_OP:
      OPT(expr->l.expr);
      if (expr->v.number == F_ASSIGN) {
        if (IS_NODE(expr->r.expr, NODE_OPCODE_1, F_LOCAL_LVALUE)) {
          if (!optimizer_state) {
            int x = expr->r.expr->l.number;

            if (last_local_refs[x]) {
              last_local_refs[x]->v.number = F_TRANSFER_LOCAL;
              last_local_refs[x] = nullptr;
            }
          }
        }
      }
      OPT(expr->r.expr);
      break;
    case NODE_UNARY_OP:
      OPT(expr->r.expr);
      break;
    case NODE_OPCODE:
      break;
    case NODE_TERNARY_OP_1:
      OPT(expr->l.expr);
      OPT(expr->r.expr->l.expr);
      OPT(expr->r.expr->r.expr);
      break;
    case NODE_BINARY_OP_1:
      OPT(expr->l.expr);
      OPT(expr->r.expr);
      break;
    case NODE_UNARY_OP_1:
      OPT(expr->r.expr);
      if (expr->v.number == F_VOID_ASSIGN_LOCAL) {
        if (last_local_refs[expr->l.number] && !optimizer_state) {
          last_local_refs[expr->l.number]->v.number = F_TRANSFER_LOCAL;
          last_local_refs[expr->l.number] = nullptr;
        }
      }
      break;
    case NODE_OPCODE_1:
      if (expr->v.number == F_LOCAL || expr->v.number == F_LOCAL_LVALUE) {
        if (expr->v.number == F_LOCAL) {
          if (!optimizer_state) {
            last_local_refs[expr->l.number] = expr;
            break;
          }
        }
        last_local_refs[expr->l.number] = nullptr;
      }
      break;
    case NODE_OPCODE_2:
      break;
    case NODE_RETURN:
      OPT(expr->r.expr);
      break;
    case NODE_STRING:
    case NODE_REAL:
    case NODE_NUMBER:
      break;
    case NODE_LAND_LOR:
    case NODE_NULLISH:
    case NODE_LOGICAL_ASSIGN:
    case NODE_BRANCH_LINK: {
      int in_cond = (optimizer_state & OPTIMIZER_IN_COND);

      OPT(expr->l.expr);
      optimizer_state |= OPTIMIZER_IN_COND;
      OPT(expr->r.expr);
      optimizer_state &= ~OPTIMIZER_IN_COND;
      optimizer_state |= in_cond;
      break;
    }
    case NODE_CALL_2:
    case NODE_CALL_1:
    case NODE_CALL:
      optimize_expr_list(expr->r.expr);
      break;
    case NODE_TWO_VALUES: {
      // Same chain, same reason to flatten it, as icode.cc's
      // i_generate_node() (see there). optimize() never replaces a
      // NODE_TWO_VALUES node's identity, so a stack of *slots* (rather
      // than values), with each leaf's optimized replacement written
      // back into its own slot, is equivalent to the recursive form.
      std::vector<parse_node_t**> work{&expr->r.expr, &expr->l.expr};
      while (!work.empty()) {
        parse_node_t** slot = work.back();
        work.pop_back();
        parse_node_t* node = *slot;
        if (node && node->kind == NODE_TWO_VALUES) {
          work.push_back(&node->r.expr);
          work.push_back(&node->l.expr);
        } else {
          OPT(*slot);
        }
      }
      break;
    }
    case NODE_CONTROL_JUMP:
    case NODE_PARAMETER:
    case NODE_PARAMETER_LVALUE:
      break;
    case NODE_IF: {
      int in_cond;
      OPT(expr->v.expr);
      in_cond = (optimizer_state & OPTIMIZER_IN_COND);
      optimizer_state |= OPTIMIZER_IN_COND;
      OPT(expr->l.expr);
      OPT(expr->r.expr);
      optimizer_state &= ~OPTIMIZER_IN_COND;
      optimizer_state |= in_cond;
      break;
    }
    case NODE_LOOP: {
      int in_loop = (optimizer_state & OPTIMIZER_IN_LOOP);
      optimizer_state |= OPTIMIZER_IN_LOOP;
      OPT(expr->v.expr);
      OPT(expr->l.expr);
      OPT(expr->r.expr);
      optimizer_state &= ~OPTIMIZER_IN_LOOP;
      optimizer_state |= in_loop;
      break;
    }
    case NODE_FOREACH:
      OPT(expr->l.expr);
      OPT(expr->r.expr);
      OPT(expr->v.expr);
      break;
    case NODE_CASE_NUMBER:
    case NODE_CASE_STRING:
    case NODE_DEFAULT:
      break;
    case NODE_SWITCH_STRINGS:
    case NODE_SWITCH_NUMBERS:
    case NODE_SWITCH_DIRECT:
    case NODE_SWITCH_RANGES: {
      int in_cond;
      OPT(expr->l.expr);
      in_cond = (optimizer_state & OPTIMIZER_IN_COND);
      optimizer_state |= OPTIMIZER_IN_COND;
      OPT(expr->r.expr);
      optimizer_state &= ~OPTIMIZER_IN_COND;
      optimizer_state |= in_cond;
      break;
    }
    case NODE_CATCH:
      OPT(expr->r.expr);
      break;
    case NODE_LVALUE_EFUN:
      OPT(expr->l.expr);
      optimize_lvalue_list(expr->r.expr);
      break;

    case NODE_FUNCTION_CONSTRUCTOR:
      /* Don't optimize inside of these; we'll get confused by local vars
       * since it's a separate frame, etc
       *
       * OPT(expr->r.expr);
       *
       * BUT make sure to optimize the things which AREN'T part of that
       * frame, namely, the arguments, otherwise we will screw up:
       *
       * use(local); return (: foo, local :);       // local evaluated at
       * use(local); return (: ... $(local) ... :); // construction time
       */
      if (expr->r.expr) {
        optimize_expr_list(expr->r.expr); /* arguments */
      }
      break;
    case NODE_ANON_FUNC:
      break;
    case NODE_EFUN:
      optimize_expr_list(expr->r.expr);
      break;
    case NODE_TIME_EXPRESSION:
      OPT(expr->r.expr);
      break;
    default:
      // This should not happen!
      // see src/testsuite/single/tests/compiler/optimize.c
      debug_message("optimizer: unknown node kind: %d.", expr->kind);
      break;
  }
  return expr;
}

ADDRESS_TYPE generate(parse_node_t* node) {
  ADDRESS_TYPE where = CURRENT_PROGRAM_SIZE;

  if (num_parse_error) {
    return 0;
  }
  {
    i_generate_node(node);
  }
  free_tree();

  return where;
}

static void optimizer_start_function(int n) {
  if (n) {
    last_local_refs = reinterpret_cast<parse_node_t**>(
        DCALLOC(n, sizeof(parse_node_t*), TAG_COMPILER, "c_start_function"));
    optimizer_num_locals = n;
    while (n--) {
      last_local_refs[n] = nullptr;
    }
  } else {
    last_local_refs = nullptr;
  }
}

static void optimizer_end_function(void) {
  int i;
  if (last_local_refs) {
    for (i = 0; i < optimizer_num_locals; i++) {
      if (last_local_refs[i]) {
        last_local_refs[i]->v.number = F_TRANSFER_LOCAL;
      }
    }
    FREE(last_local_refs);
    last_local_refs = nullptr;
  }
}

ADDRESS_TYPE generate_function(function_t* f, parse_node_t* node, int num) {
  ADDRESS_TYPE ret;
  if (pragmas & PRAGMA_OPTIMIZE) {
    optimizer_start_function(num);
    optimizer_state = 0;
    node = optimize(node);
    optimizer_end_function();
  }
  ret = generate(node);
  return ret;
}

int node_always_true(parse_node_t* node) {
  if (!node) {
    return 1;
  }
  if (node->kind == NODE_NUMBER) {
    return node->v.number;
  }
  return 0;
}

int generate_conditional_branch(parse_node_t* node) {
  int branch;

  if (!node) {
    return F_BBRANCH;
  }

  /* only have to handle while (x != 0) since while (x == 0) will be
   * handled by the x == 0 -> !x and !x optimizations.
   */
  if (IS_NODE(node, NODE_BINARY_OP, F_NE)) {
    if (IS_NODE(node->r.expr, NODE_NUMBER, 0)) {
      node = node->l.expr;
    } else if (IS_NODE(node->l.expr, NODE_NUMBER, 0)) {
      node = node->r.expr;
    }
  }
  if (IS_NODE(node, NODE_UNARY_OP, F_NOT)) {
    node = node->r.expr;
    branch = F_BBRANCH_WHEN_ZERO;
  } else {
    branch = F_BBRANCH_WHEN_NON_ZERO;
    if (node->kind == NODE_NUMBER) {
      if (node->v.number == 0) {
        branch = 0;
      } else {
        branch = F_BBRANCH;
      }
      node = nullptr;
    }
    if (node) {
      if (IS_NODE(node, NODE_BINARY_OP, F_LT)) {
        generate(node->l.expr);
        generate(node->r.expr);
        return F_BBRANCH_LT;
      }
      if (IS_NODE(node, NODE_OPCODE_1, F_WHILE_DEC)) {
        generate(node);
        return F_WHILE_DEC;
      }
    }
  }
  generate(node);
  return branch;
}

// FIXME: originally found in generate.cc

// (Unconditional: lpcc --ast ships dump_tree in every build type.)
const char* lpc_tree_name[] = {"return",
                               "two values",
                               "opcode",
                               "opcode_1",
                               "opcode_2",
                               "unary op",
                               "unary op_1",
                               "binary op",
                               "binary op_1",
                               "ternary op",
                               "ternary op_1",
                               "control jump",
                               "loop",
                               "call",
                               "call_1",
                               "call_2",
                               "&& ||",
                               "nullish",
                               "logical assign",
                               "foreach",
                               "lvalue_efun",
                               "switch_range",
                               "switch_string",
                               "switch_direct",
                               "switch_number",
                               "case_number",
                               "case_string",
                               "default",
                               "if",
                               "branch link",
                               "parameter",
                               "parameter_lvalue",
                               "efun",
                               "anon func",
                               "real",
                               "number",
                               "string",
                               "function",
                               "catch"};

static void lpc_tree(parse_node_t* dest, int num) {
  parse_node_t* pn;

  dest->kind = NODE_CALL;
  dest->v.number = F_AGGREGATE;
  dest->type = TYPE_ANY | TYPE_MOD_ARRAY;
  dest->l.number = num;
  if (!num) {
    dest->r.expr = 0;
  } else {
    dest->r.expr = new_node_no_line();
    dest->r.expr->kind = num--;
    pn = dest->r.expr;
    while (num--) {
      pn->r.expr = new_node_no_line();
      pn->type = 0;
      pn = pn->r.expr;
    }
    pn->type = 0;
    pn->r.expr = 0;
    dest->r.expr->l.expr = pn;
  }
}

static void lpc_tree_number(parse_node_t* dest, LPC_INT num) { CREATE_NUMBER(dest->v.expr, num); }

static void lpc_tree_real(parse_node_t* dest, LPC_FLOAT real) { CREATE_REAL(dest->v.expr, real); }

static void lpc_tree_expr(parse_node_t* dest, parse_node_t* expr) {
  dest->v.expr = new_node_no_line();
  lpc_tree_form(expr, dest->v.expr);
}

static void lpc_tree_string(parse_node_t* dest, const char* str) {
  CREATE_STRING(dest->v.expr, str);
}

static void lpc_tree_list(parse_node_t* dest, parse_node_t* expr) {
  parse_node_t* pn;
  int num = 0;

  pn = expr;
  while (pn) {
    pn = pn->r.expr;
    num++;
  }

  dest->v.expr = new_node_no_line();
  lpc_tree(dest->v.expr, num);
  dest = dest->v.expr;
  while (expr) {
    dest = dest->r.expr;
    lpc_tree_expr(dest, expr->v.expr);
    expr = expr->r.expr;
  }
}

#define lpc_tree_opc(x, y) lpc_tree_string(x, query_instr_name(y & ~NOVALUE_USED_FLAG))

#define ARG_1 dest->r.expr
#define ARG_2 dest->r.expr->r.expr
#define ARG_3 dest->r.expr->r.expr->r.expr
#define ARG_4 dest->r.expr->r.expr->r.expr->r.expr
#define ARG_5 dest->r.expr->r.expr->r.expr->r.expr->r.expr

// dump_tree (lpcc --ast text form) renders the SAME model the --json mode
// emits (ast_json below): one switch, two renderings -- the S-expression
// text is `(label scalars children...)` with seq nodes spliced flat, which
// is shape-compatible with the historical hand-printed form.
static void render_sexpr(const nlohmann::json& n);

static void render_sexpr_children(const nlohmann::json& n, bool* first) {
  if (n.contains("a")) {
    for (const auto& a : n["a"]) {
      if (!*first) printf(" ");
      *first = false;
      if (a.is_string()) {
        printf("%s", a.get<std::string>().c_str());
      } else {
        printf("%s", a.dump().c_str());
      }
    }
  }
  if (n.contains("c")) {
    for (const auto& c : n["c"]) {
      if (!*first) printf(" ");
      *first = false;
      render_sexpr(c);
    }
  }
}

static void render_sexpr(const nlohmann::json& n) {
  const std::string k = n.value("k", "");
  if (k == "seq") {
    // sequencing node: splice children flat, like the historical dump
    bool first = true;
    if (n.contains("c")) {
      for (const auto& c : n["c"]) {
        if (!first) printf(" ");
        first = false;
        render_sexpr(c);
      }
    }
    return;
  }
  printf("(%s", k.c_str());
  bool firstitem = false;  // label already printed; separate items with ' '
  render_sexpr_children(n, &firstitem);
  printf(")");
}

// --- lpcc --ast --json ------------------------------------------------------
//
// Structured mirror of dump_tree. Node schema (keys omitted when empty):
//   {"k": label, "l": source line, "a": [scalar operands], "c": [children]}
// NODE_TWO_VALUES is a pure sequencing node; it renders as {"k":"seq"} with
// nested sequences spliced flat, matching the S-expression form's implicit
// concatenation. Kept adjacent to dump_tree ON PURPOSE: any new NODE_* case
// added there must be added here (both are driven by the same switch shape).

static nlohmann::json ast_json(parse_node_t* expr);

static void ast_json_children(nlohmann::json& node, parse_node_t* expr) {
  // dump_expr_list: right-linked list of v.expr items
  while (expr) {
    node["c"].push_back(ast_json(expr->v.expr));
    expr = expr->r.expr;
  }
}

static void ast_json_seq(nlohmann::json& arr, parse_node_t* expr) {
  // Same NODE_TWO_VALUES chain (one node per top-level definition/statement)
  // that i_generate_node()/optimize() flatten iteratively instead of
  // recursing -- walking it recursively here would scale C-stack depth with
  // an object's definition/statement count, not just genuine expression
  // nesting (github.com/fluffos/fluffos/issues/1267). Push right-then-left
  // so popping (left-to-right) preserves the original recursive order.
  std::vector<parse_node_t*> work{expr};
  while (!work.empty()) {
    parse_node_t* node = work.back();
    work.pop_back();
    if (!node) continue;
    if (node->kind == NODE_TWO_VALUES) { // splice nested sequences flat
      work.push_back(node->r.expr);
      work.push_back(node->l.expr);
    } else {
      arr.push_back(ast_json(node));
    }
  }
}

namespace {
// Mirrors optimize()'s guard (see above): ast_json() is a third recursive
// walker over the same parse-tree shape as optimize()/i_generate_node(), for
// `lpcc --ast`/`--ast --json`, and runs BEFORE codegen -- so without its own
// cap it can stack-overflow on a pathologically deep (non-constant-foldable)
// expression before optimize()'s own cap ever gets a chance to reject it.
// Diagnostic output only, so going over just truncates the subtree instead
// of failing anything.
int g_ast_json_depth = 0;
constexpr int kMaxAstJsonDepth = 500;
}  // namespace

static nlohmann::json ast_json(parse_node_t* expr) {
  nlohmann::json n = nlohmann::json::object();
  if (!expr) {
    n["k"] = "nil";
    return n;
  }
  if (++g_ast_json_depth > kMaxAstJsonDepth) {
    --g_ast_json_depth;
    n["k"] = "too_deep";
    return n;
  }
  DEFER { --g_ast_json_depth; };
  if (expr->line > 0) n["l"] = expr->line;
  auto kids = [&](parse_node_t* e) { if (e) n["c"].push_back(ast_json(e)); };
  auto scalar = [&](LPC_INT v) { n["a"].push_back(v); };
  const auto instr_name = [](int op) { return instrs[op & ~NOVALUE_USED_FLAG].name; };

  switch (expr->kind) {
    case NODE_TERNARY_OP:
    case NODE_TERNARY_OP_1:
      n["k"] = instr_name(expr->r.expr->v.number);
      if (expr->kind == NODE_TERNARY_OP_1) scalar(expr->type);
      kids(expr->l.expr);
      kids(expr->r.expr->l.expr);
      kids(expr->r.expr->r.expr);
      break;
    case NODE_BINARY_OP:
    case NODE_BINARY_OP_1:
      n["k"] = instr_name(expr->v.number);
      if (expr->kind == NODE_BINARY_OP_1) scalar(expr->type);
      kids(expr->l.expr);
      kids(expr->r.expr);
      break;
    case NODE_UNARY_OP:
      n["k"] = instr_name(expr->v.number);
      kids(expr->r.expr);
      break;
    case NODE_UNARY_OP_1:
      n["k"] = instr_name(expr->v.number);
      scalar(expr->l.number);
      kids(expr->r.expr);
      break;
    case NODE_OPCODE:
      n["k"] = instr_name(expr->v.number);
      break;
    case NODE_OPCODE_1:
      n["k"] = instr_name(expr->v.number);
      scalar(expr->l.number);
      break;
    case NODE_OPCODE_2:
      n["k"] = instr_name(expr->v.number);
      scalar(expr->l.number);
      scalar(expr->r.number);
      break;
    case NODE_RETURN:
      n["k"] = expr->r.expr ? "return" : "return_zero";
      kids(expr->r.expr);
      break;
    case NODE_STRING:
      n["k"] = "string";
      scalar(expr->v.number); // string-table index; resolve via dump_prog STRINGS
      break;
    case NODE_REAL:
      n["k"] = "real";
      n["a"].push_back(expr->v.real);
      break;
    case NODE_NUMBER:
      n["k"] = "number";
      scalar(expr->v.number);
      break;
    case NODE_LAND_LOR:
      n["k"] = (expr->v.number == F_LAND) ? "&&" : "||";
      kids(expr->l.expr);
      kids(expr->r.expr);
      break;
    case NODE_NULLISH:
      n["k"] = "??";
      kids(expr->l.expr);
      kids(expr->r.expr);
      break;
    case NODE_LOGICAL_ASSIGN:
    case NODE_BRANCH_LINK:
      n["k"] = expr->kind == NODE_BRANCH_LINK ? "branch_link" : instr_name(expr->v.number);
      kids(expr->l.expr);
      kids(expr->r.expr);
      break;
    case NODE_CALL_2:
      n["k"] = instr_name(expr->v.number);
      scalar(expr->l.number >> 16);
      scalar(expr->l.number & 0xffff);
      ast_json_children(n, expr->r.expr);
      break;
    case NODE_CALL_1:
      n["k"] = instr_name(expr->v.number);
      scalar(expr->l.number);
      ast_json_children(n, expr->r.expr);
      break;
    case NODE_CALL:
      n["k"] = instr_name(expr->v.number);
      scalar(expr->l.number);
      ast_json_children(n, expr->r.expr);
      break;
    case NODE_TWO_VALUES:
      n["k"] = "seq";
      ast_json_seq(n["c"], expr->l.expr);
      ast_json_seq(n["c"], expr->r.expr);
      break;
    case NODE_CONTROL_JUMP:
      n["k"] = (expr->v.number == CJ_BREAK_SWITCH) ? "break_switch"
               : (expr->v.number == CJ_BREAK)      ? "break"
               : (expr->v.number == CJ_CONTINUE)   ? "continue"
                                                   : "unknown_control_jump";
      break;
    case NODE_PARAMETER:
      n["k"] = "parameter";
      scalar(expr->v.number);
      break;
    case NODE_PARAMETER_LVALUE:
      n["k"] = "parameter_lvalue";
      scalar(expr->v.number);
      break;
    case NODE_IF:
      n["k"] = "if";
      kids(expr->v.expr);
      kids(expr->l.expr);
      kids(expr->r.expr);
      break;
    case NODE_LOOP:
      n["k"] = "loop";
      scalar(expr->type);
      kids(expr->v.expr);
      kids(expr->l.expr);
      kids(expr->r.expr);
      break;
    case NODE_FOREACH:
      n["k"] = "foreach";
      kids(expr->l.expr);
      kids(expr->r.expr);
      kids(expr->v.expr);
      break;
    case NODE_CASE_NUMBER:
    case NODE_CASE_STRING:
      n["k"] = "case";
      break;
    case NODE_DEFAULT:
      n["k"] = "default";
      break;
    case NODE_SWITCH_STRINGS:
    case NODE_SWITCH_NUMBERS:
    case NODE_SWITCH_DIRECT:
    case NODE_SWITCH_RANGES:
      n["k"] = "switch";
      kids(expr->l.expr);
      kids(expr->r.expr);
      break;
    case NODE_CATCH:
      n["k"] = "catch";
      kids(expr->r.expr);
      break;
    case NODE_LVALUE_EFUN: {
      n["k"] = "lvalue_efun";
      kids(expr->l.expr);
      parse_node_t* lv = expr->r.expr;
      while (lv && (lv = lv->r.expr)) n["c"].push_back(ast_json(lv->l.expr));
      break;
    }
    case NODE_FUNCTION_CONSTRUCTOR: {
      n["k"] = "functional";
      scalar(expr->v.number & 0xff);
      scalar(expr->v.number >> 8);
      if (expr->r.expr) ast_json_children(n, expr->r.expr);
      switch (expr->v.number & 0xff) {
        case FP_EFUN:
          // At AST time v.number>>8 is a predefs[] index (icode translates
          // it to the instruction via predefs[idx].token when emitting).
          // Resolving it against instrs[] directly printed an unrelated,
          // build-dependent opcode name for every (: efun :) node.
          n["a"].push_back(predefs[expr->v.number >> 8].word);
          break;
        case FP_FUNCTIONAL:
        case FP_FUNCTIONAL | FP_NOT_BINDABLE:
          kids(expr->l.expr);
          break;
      }
      break;
    }
    case NODE_ANON_FUNC:
      n["k"] = "anon_func";
      scalar(expr->v.number);
      scalar(expr->l.number);
      kids(expr->r.expr);
      break;
    case NODE_EFUN:
      n["k"] = "efun";
      n["a"].push_back(instr_name(expr->v.number));
      ast_json_children(n, expr->r.expr);
      break;
    case NODE_FUNCTION:
      n["k"] = "function";
      scalar(expr->v.number); // function index
      kids(expr->r.expr);
      break;
    default:
      n["k"] = "unknown";
      scalar(expr->kind);
      break;
  }
  return n;
}

void dump_tree(parse_node_t* expr) {
  nlohmann::json roots = nlohmann::json::array();
  ast_json_seq(roots, expr);
  bool first = true;
  for (const auto& r : roots) {
    if (!first) printf("\n");
    first = false;
    render_sexpr(r);
  }
}

void dump_program_ast_json(const char* filename, parse_node_t* tree_main,
                           parse_node_t* tree_init) {
  nlohmann::json envelope = {
      {"fluffos_lpcc", 1},
      {"stage", "ast"},
      {"file", filename != nullptr ? filename : "?"},
      {"trees", nlohmann::json::array()},
  };
  nlohmann::json main_roots = nlohmann::json::array();
  ast_json_seq(main_roots, tree_main);
  nlohmann::json init_roots = nlohmann::json::array();
  ast_json_seq(init_roots, tree_init);
  envelope["trees"].push_back({{"title", "TREE_MAIN"}, {"roots", main_roots}});
  envelope["trees"].push_back({{"title", "TREE_INIT"}, {"roots", init_roots}});
  // Filenames/instr names are ASCII, but stay consistent with the other
  // envelopes: never throw on stray bytes.
  printf("%s\n",
         envelope.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace).c_str());
}

void lpc_tree_form(parse_node_t* expr, parse_node_t* dest) {
  if (!expr) {
    dest->kind = NODE_NUMBER;
    dest->type = TYPE_ANY;
    dest->v.number = 0;
    return;
  }

  switch (expr->kind) {
    case NODE_TERNARY_OP:
      lpc_tree(dest, 4);
      lpc_tree_opc(ARG_2, expr->r.expr->v.number);
      lpc_tree_expr(ARG_3, expr->l.expr);
      lpc_tree_expr(ARG_4, expr->r.expr);
      break;
    case NODE_TERNARY_OP_1:
      lpc_tree(dest, 5);
      lpc_tree_opc(ARG_2, expr->r.expr->v.number);
      lpc_tree_number(ARG_3, expr->type);
      lpc_tree_expr(ARG_4, expr->l.expr);
      lpc_tree_expr(ARG_5, expr->r.expr);
      break;
    case NODE_BINARY_OP:
    case NODE_LAND_LOR:
    case NODE_LOGICAL_ASSIGN:
    case NODE_NULLISH:
    case NODE_BRANCH_LINK:
      lpc_tree(dest, 4);
      lpc_tree_opc(ARG_2, expr->v.number);
      lpc_tree_expr(ARG_3, expr->l.expr);
      lpc_tree_expr(ARG_4, expr->r.expr);
      break;
    case NODE_TWO_VALUES:
      lpc_tree(dest, 3);
      lpc_tree_expr(ARG_2, expr->l.expr);
      lpc_tree_expr(ARG_3, expr->r.expr);
      break;
    case NODE_BINARY_OP_1:
      lpc_tree(dest, 5);
      lpc_tree_opc(ARG_2, expr->v.number);
      lpc_tree_number(ARG_3, expr->type);
      lpc_tree_expr(ARG_4, expr->l.expr);
      lpc_tree_expr(ARG_5, expr->r.expr);
      break;
    case NODE_UNARY_OP:
      lpc_tree(dest, 3);
      lpc_tree_opc(ARG_2, expr->v.number);
      lpc_tree_expr(ARG_3, expr->r.expr);
      break;
    case NODE_UNARY_OP_1:
      lpc_tree(dest, 4);
      lpc_tree_opc(ARG_2, expr->v.number);
      lpc_tree_number(ARG_3, expr->l.number);
      lpc_tree_expr(ARG_4, expr->r.expr);
      break;
    case NODE_OPCODE:
      lpc_tree(dest, 2);
      lpc_tree_opc(ARG_2, expr->v.number);
      break;
    case NODE_CONTROL_JUMP:
    case NODE_PARAMETER:
    case NODE_PARAMETER_LVALUE:
      lpc_tree(dest, 2);
      lpc_tree_number(ARG_2, expr->v.number);
      break;
    case NODE_OPCODE_1:
      lpc_tree(dest, 3);
      lpc_tree_opc(ARG_2, expr->v.number);
      lpc_tree_number(ARG_3, expr->l.number);
      break;
    case NODE_OPCODE_2:
      lpc_tree(dest, 4);
      lpc_tree_opc(ARG_2, expr->v.number);
      lpc_tree_number(ARG_3, expr->l.number);
      lpc_tree_number(ARG_4, expr->r.number);
      break;
    case NODE_RETURN:
      lpc_tree(dest, 2);
      lpc_tree_expr(ARG_2, expr->r.expr);
      break;
    case NODE_STRING:
    case NODE_NUMBER:
      lpc_tree(dest, 2);
      lpc_tree_number(ARG_2, expr->v.number);
      break;
    case NODE_REAL:
      lpc_tree(dest, 2);
      lpc_tree_real(ARG_2, expr->v.real);
      break;
    case NODE_CALL_2:
    case NODE_CALL_1:
    case NODE_CALL:
      lpc_tree(dest, 4);
      lpc_tree_opc(ARG_2, expr->v.number);
      lpc_tree_list(ARG_3, expr->r.expr);
      lpc_tree_number(ARG_4, expr->l.number);
      break;
    case NODE_IF:
    case NODE_FOREACH:
      lpc_tree(dest, 4);
      lpc_tree_expr(ARG_2, expr->l.expr);
      lpc_tree_expr(ARG_3, expr->r.expr);
      lpc_tree_expr(ARG_4, expr->v.expr);
      break;
    case NODE_LOOP:
      lpc_tree(dest, 5);
      lpc_tree_number(ARG_2, expr->type);
      lpc_tree_expr(ARG_3, expr->v.expr);
      lpc_tree_expr(ARG_4, expr->r.expr);
      lpc_tree_expr(ARG_5, expr->l.expr);
      break;
    case NODE_CASE_NUMBER:
    case NODE_CASE_STRING:
    case NODE_DEFAULT:
      lpc_tree(dest, 1);
      break;
    case NODE_SWITCH_STRINGS:
    case NODE_SWITCH_NUMBERS:
    case NODE_SWITCH_DIRECT:
    case NODE_SWITCH_RANGES:
      lpc_tree(dest, 3);
      lpc_tree_expr(ARG_2, expr->l.expr);
      lpc_tree_expr(ARG_3, expr->r.expr);
      break;
    case NODE_CATCH:
      lpc_tree(dest, 2);
      lpc_tree_expr(ARG_2, expr->r.expr);
      break;
    case NODE_LVALUE_EFUN:
      lpc_tree(dest, 3);
      lpc_tree_expr(ARG_2, expr->l.expr);
      lpc_tree_list(ARG_3, expr->r.expr);
      break;
    case NODE_FUNCTION_CONSTRUCTOR:
    case NODE_EFUN:
      lpc_tree(dest, 3);
      lpc_tree_opc(ARG_2, expr->v.number);
      lpc_tree_list(ARG_3, expr->r.expr);
      break;
    default:
      lpc_tree(dest, 1);
      lpc_tree_string(ARG_1, "!GARBAGE!");
      return;
  }
  lpc_tree_string(ARG_1, lpc_tree_name[expr->kind]);
}
