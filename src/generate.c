#include "generate.h"
#include "opcodes.h"
#include "compiler_shared.h"
#include "fp_defs.h"

void dump_tree PROT((struct parse_node *, int));
static struct parse_node *optimize_expr PROT((struct parse_node *));

#ifdef LPC_TO_C
#  define DISPATCH(x) void x() { if (compile_to_c) c_##x(); else i_##x(); }
#  define DISPATCH1(x, a, b) void x P1(a,b) { if (compile_to_c) c_##x(b); else i_##x(b); }
#  define DISPATCH2(x, a, b, c, d) void x P2(a,b,c,d) { if (compile_to_c) c_##x(b, d); else i_##x(b,d); }
#else
#  define DISPATCH(x) void x() { i_##x(); }
#  define DISPATCH1(x, a, b) void x P1(a, b) { i_##x(b); }
#  define DISPATCH2(x, a, b, c, d) void x P2(a, b, c, d) { i_##x(b, d); }
#endif

static void
optimize_expr_list P1(struct parse_node *, expr) {
    if (!expr) return;
    do {
	optimize_expr(expr->v.expr);
    } while (expr = expr->right);
}

static void
optimize_lvalue_list P1(struct parse_node *, expr) {
    while (expr = expr->right) {
	optimize_expr(expr->left);
    }
}

static struct parse_node *
optimize_expr P1(struct parse_node *, expr) {
    if (!expr) return 0;
    switch (expr->kind) {
    case F_RANGE:
	expr->v.expr = optimize_expr(expr->v.expr);
	/* fall through */
    case F_LOR:
    case F_LAND:
    case F_ASSIGN:
    case F_VOID_ASSIGN:
    case F_VOID_ADD_EQ:
    case F_ADD_EQ:
    case F_AND_EQ:
    case F_OR_EQ:
    case F_XOR_EQ:
    case F_LSH_EQ:
    case F_RSH_EQ:
    case F_SUB_EQ:
    case F_MULT_EQ:
    case F_MOD_EQ:
    case F_DIV_EQ:
    case F_INDEXED_LVALUE:
    case F_INDEX:
    case F_OR:
    case F_XOR:
    case F_AND:
    case F_EQ:
    case F_NE:
    case F_GT:
    case F_GE:
    case F_LT:
    case F_LE:
    case F_LSH:
    case F_RSH:
    case F_ADD:
    case F_SUBTRACT:
    case F_MULTIPLY:
    case F_DIVIDE:
    case F_MOD:
    case NODE_COMMA:
    case NODE_ASSOC:
	expr->left = optimize_expr(expr->left);
	/* fall through */
    case F_POP_VALUE:
    case F_PRE_INC:
    case F_PRE_DEC:
    case F_INC:
    case F_DEC:
    case F_POST_INC:
    case F_POST_DEC:
    case F_NOT:
    case F_COMPL:
    case F_NEGATE:
    case F_CATCH:
    case F_TIME_EXPRESSION:
	expr->right = optimize_expr(expr->right);
	break;
    case F_AGGREGATE:
    case F_AGGREGATE_ASSOC:
    case F_TO_FLOAT:
    case F_TO_INT:
    case F_SIMUL_EFUN:
    case F_CALL_FUNCTION_BY_ADDRESS:
	optimize_expr_list(expr->right);
	break;
    case NODE_CONDITIONAL:
	expr->left = optimize_expr(expr->left);
	expr->right->left = optimize_expr(expr->right->left);
	expr->right->right = optimize_expr(expr->right->right);
	break;
    case F_SSCANF:
	expr->left->left = optimize_expr(expr->left->left);
	expr->left->right = optimize_expr(expr->left->right);
	optimize_lvalue_list(expr->right);
	break;
    case F_PARSE_COMMAND:
	expr->left->left = optimize_expr(expr->left->left);
	expr->left->right->left = optimize_expr(expr->left->right->left);
	expr->left->right->right = optimize_expr(expr->left->right->right);
	optimize_lvalue_list(expr->right);
	break;
    case F_EVALUATE:
#ifdef NEW_FUNCTIONS
	optimize_expr_list(expr->right);
#else
	expr->left = optimize_expr(expr->left);
	expr->right = optimize_expr(expr->right);
#endif
	break;
    case F_FUNCTION_CONSTRUCTOR:
#ifdef NEW_FUNCTIONS
	if ((expr->v.number & 0xff) == FP_CALL_OTHER) {
	    expr->left = optimize_expr(expr->left);
	    expr->right = optimize_expr(expr->right);
	    break;
	}
	if (expr->right)
	    optimize_expr_list(expr->right);
	if (expr->v.number & 0xff == FP_FUNCTIONAL)
	    expr->left = optimize_expr(expr->left);
#else
	if (expr->left)
	    expr->left = optimize_expr(expr->left);
	expr->right = optimize_expr(expr->right);
#endif
	break;
    default:
	if (expr->kind > BASE)
	    optimize_expr_list(expr->right);
    }
    return expr;
}

void
generate_expr P1(struct parse_node *, node) {
  if (num_parse_error) return;
  if (pragmas & PRAGMA_OPTIMIZE) node = optimize_expr(node);
#ifdef LPC_TO_C
  if (compile_to_c)
    c_generate_node(node);
  else
#endif
    i_generate_node(node);
  free_tree();
}

int
node_always_true P1(struct parse_node *, node) {
    if (node->flags & E_CONST) {
	if (node->kind == F_NUMBER && node->v.number == 0)
	    return 0;
	return 1;
    }
    return 0;
}

void
generate_conditional_branch P1(struct parse_node *, node) {
    int branch;

    /* only have to handle while (x != 0) since while (x == 0) will be
     * handled by the x == 0 -> !x and !x optimizations.
     */
    if (node->kind == F_NE) {
	if (node->right->kind == F_NUMBER && node->right->v.number == 0)
	    node = node->left;
	else if (node->left->kind == F_NUMBER && node->left->v.number == 0)
	    node = node->right;
    }
    if (node->kind == F_NOT) {
	node = node->right;
	branch = F_BBRANCH_WHEN_ZERO;
    } else {
	branch = F_BBRANCH_WHEN_NON_ZERO;
	if (node->flags & E_CONST) {
	    if (node->kind == F_NUMBER && node->v.number == 0)
		branch = 0;
	    else branch = F_BBRANCH;
	    node = 0;
	}
    }
    generate_expr(node);
    branch_backwards(branch);
}

DISPATCH2(generate_function_call, short, f, char, num)

DISPATCH(pop_value)

DISPATCH(generate___INIT)

DISPATCH1(generate_final_program, int, flag)

DISPATCH1(generate_return, struct parse_node *, node)

DISPATCH(generate_break_point)

DISPATCH(generate_break)

DISPATCH(generate_continue)

DISPATCH(update_continues)

DISPATCH(save_position)

DISPATCH1(branch_backwards, char, b)

DISPATCH(update_breaks)

DISPATCH(save_loop_info)

DISPATCH(restore_loop_info)

DISPATCH(start_switch)

DISPATCH1(generate_forward_branch, char, b)

DISPATCH(update_forward_branch)

DISPATCH(generate_else)

DISPATCH(initialize_parser)

#ifdef DEBUG
void
dump_expr_list P2(struct parse_node *, expr, int, indent) {
    if (!expr) return;
    do {
      dump_tree(expr->v.expr, indent);
    } while (expr = expr->right);
}

static void
dump_lvalue_list P2(struct parse_node *, expr, int, indent) {
    int i;

    for (i=0; i<indent; i++) 
        putchar(' ');
    printf("lvalue_list\n");
    while (expr = expr->right)
      dump_tree(expr->left, indent + 2);
}

void
dump_tree P2(struct parse_node *, expr, int, indent) {
    int i;
    if (!expr) return;
    for (i=0; i<indent; i++) 
        putchar(' ');
    switch (expr->kind) {
    case NODE_ASSOC:
	dump_tree(expr->left, indent + 2);
	dump_tree(expr->right, indent + 2);
	break;
    case NODE_CONDITIONAL:
      printf("?\n");
      dump_tree(expr->left, indent + 2);
      for (i=0; i<indent + 2; i++) 
        putchar(' ');
      printf(":\n");
      dump_tree(expr->right->left, indent + 4);
      dump_tree(expr->right->right, indent + 4);
      break;
    case 0:
      printf("<empty>\n");
      dump_tree(expr->left, indent + 2);
      dump_tree(expr->right, indent + 2);
      break;
    case F_RANGE:
      printf("%s\n", instrs[expr->kind].name);
      dump_tree(expr->v.expr, indent + 2);
      dump_tree(expr->left, indent + 2);
      dump_tree(expr->right, indent + 2);
      break;
    case F_INDEX:
    case F_INDEXED_LVALUE:
    case F_ADD:
    case F_SUBTRACT:
    case F_MULTIPLY:
    case F_DIVIDE:
    case F_MOD:
    case F_VOID_ASSIGN:
    case F_EQ:
    case F_NE:
    case F_LAND:
    case F_AND:
    case F_OR:
    case F_XOR:
    case F_LOR:
    case F_ASSIGN:
    case F_ADD_EQ:
    case F_SUB_EQ:
    case F_DIV_EQ:
    case F_MULT_EQ:
    case F_GT:
    case F_GE:
    case F_LT:
    case F_LE:
    case F_VOID_ADD_EQ:
      printf("%s\n", instrs[expr->kind].name);
      dump_tree(expr->left, indent + 2);
      dump_tree(expr->right, indent + 2);
      break;
    case F_POP_VALUE:
    case F_NOT:
    case F_CATCH:
    case F_NEGATE:
    case F_COMPL:
    case F_INC:
    case F_DEC:
    case F_PRE_INC:
    case F_PRE_DEC:
    case F_POST_INC:
    case F_POST_DEC:
    case F_WHILE_DEC:
      printf("%s\n", instrs[expr->kind].name);
      dump_tree(expr->right, indent + 2);
      break;
    case F_SSCANF:
      printf("sscanf\n");
      dump_tree(expr->left, indent + 2);
      dump_lvalue_list(expr->right, indent + 2);
      break;
    case F_REAL:
      printf("%f\n", expr->v.real);
      break;
    case F_NUMBER:
      printf("%d\n", expr->v.number);
      break;
    case F_CONST0:
      printf("0\n");
      break;
    case F_CONST1:
      printf("1\n");
      break;
    case F_AGGREGATE:
      printf("aggregate\n");
      dump_expr_list(expr->right, indent + 2);
      break;
    case F_AGGREGATE_ASSOC:
      printf("aggregate_assoc\n");
      dump_expr_list(expr->right, indent + 2);
      break;
    case F_CALL_FUNCTION_BY_ADDRESS:
      printf("call function %x\n", expr->v.number);
      dump_expr_list(expr->right, indent + 2);
      break;
    case F_STRING:
      printf("string \"%s\"\n", ((char **)mem_block[A_STRINGS].block)[expr->v.number]);
      break;
    case F_LOCAL_LVALUE:
    case F_GLOBAL_LVALUE:
    case F_LOCAL:
    case F_GLOBAL:
    case F_LOOP_INCR:
    case F_SIMUL_EFUN:
      printf("%s %i\n", instrs[expr->kind].name, expr->v.number);
      break;
    case F_LOOP_COND:
      printf("loop condition: local %i < ", expr->left->v.number);
      if (expr->right->kind == F_NUMBER) {
	printf("%i\n", expr->right->v.number);
      } else {
	printf("local %i\n", expr->right->v.number);
      }
      break;
    default:
      if (expr->kind >= BASE) {
	if (expr->v.number != -1)
	  printf("%s %i\n", instrs[expr->kind].name, expr->v.number);
	else
	  printf("%s\n", instrs[expr->kind].name);
	dump_expr_list(expr->right, indent + 2);
      } else {
	fatal("Unknown node type %d in dump tree.\n", expr->kind);
      }
    }
  }
#endif
