#include "std.h"
#include "generate.h"
#include "compiler.h"
#include "icode.h"
#include "lex.h"
#include "simulate.h"
#include "include/function.h"

void dump_tree PROT((struct parse_node *, int));
static struct parse_node *optimize PROT((struct parse_node *));

static void
optimize_expr_list P1(struct parse_node *, expr) {
    if (!expr) return;
    do {
	optimize(expr->v.expr);
    } while (expr = expr->r.expr);
}

static void
optimize_lvalue_list P1(struct parse_node *, expr) {
    while (expr = expr->r.expr) {
	optimize(expr->l.expr);
    }
}

static struct parse_node *
optimize P1(struct parse_node *, expr) {
    if (!expr) return 0;
    switch (expr->kind) {
    case F_NN_RANGE:
    case F_RN_RANGE:
    case F_RR_RANGE:
    case F_NR_RANGE:
    case F_NE_RANGE:
    case F_RE_RANGE:
	expr->v.expr = optimize(expr->v.expr);
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
    case F_INDEX_LVALUE:
    case F_INDEX:
    case F_RINDEX:
    case F_RINDEX_LVALUE:
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
	expr->l.expr = optimize(expr->l.expr);
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
	expr->r.expr = optimize(expr->r.expr);
	break;
    case F_AGGREGATE:
    case F_AGGREGATE_ASSOC:
    case F_TO_FLOAT:
    case F_TO_INT:
    case F_SIMUL_EFUN:
    case F_CALL_FUNCTION_BY_ADDRESS:
	optimize_expr_list(expr->r.expr);
	break;
    case NODE_CONDITIONAL:
	expr->l.expr = optimize(expr->l.expr);
	expr->r.expr->l.expr = optimize(expr->r.expr->l.expr);
	expr->r.expr->r.expr = optimize(expr->r.expr->r.expr);
	break;
    case F_SSCANF:
	expr->l.expr->l.expr = optimize(expr->l.expr->l.expr);
	expr->l.expr->r.expr = optimize(expr->l.expr->r.expr);
	optimize_lvalue_list(expr->r.expr);
	break;
    case F_PARSE_COMMAND:
	expr->l.expr->l.expr = optimize(expr->l.expr->l.expr);
	expr->l.expr->r.expr->l.expr = optimize(expr->l.expr->r.expr->l.expr);
	expr->l.expr->r.expr->r.expr = optimize(expr->l.expr->r.expr->r.expr);
	optimize_lvalue_list(expr->r.expr);
	break;
    case F_EVALUATE:
#ifdef NEW_FUNCTIONS
	optimize_expr_list(expr->r.expr);
#else
	expr->l.expr = optimize(expr->l.expr);
	expr->r.expr = optimize(expr->r.expr);
#endif
	break;
    case F_FUNCTION_CONSTRUCTOR:
#ifdef NEW_FUNCTIONS
	if ((expr->v.number & 0xff) == FP_CALL_OTHER) {
	    expr->l.expr = optimize(expr->l.expr);
	    expr->r.expr = optimize(expr->r.expr);
	    break;
	}
	if (expr->r.expr)
	    optimize_expr_list(expr->r.expr);
	if ((expr->v.number & 0xff) == FP_FUNCTIONAL)
	    expr->l.expr = optimize(expr->l.expr);
#else
	if (expr->l.expr)
	    expr->l.expr = optimize(expr->l.expr);
	expr->r.expr = optimize(expr->r.expr);
#endif
	break;
    default:
	if (expr->kind > BASE)
	    optimize_expr_list(expr->r.expr);
    }
    return expr;
}

short
generate P1(struct parse_node *, node) {
    short where = CURRENT_PROGRAM_SIZE;

    if (num_parse_error) return 0;
    if (pragmas & PRAGMA_OPTIMIZE) node = optimize(node);
#ifdef LPC_TO_C
    if (compile_to_c)
	c_generate_node(node);
    else
#endif
    {
	i_generate_node(node);
    }
    free_tree();
    
    return where;
}

int
node_always_true P1(struct parse_node *, node) {
    if (node->kind == F_NUMBER)
	return node->v.number;
    return 0;
}

int
generate_conditional_branch P1(struct parse_node *, node) {
    int branch;

    /* only have to handle while (x != 0) since while (x == 0) will be
     * handled by the x == 0 -> !x and !x optimizations.
     */
    if (node->kind == F_NE) {
	if (node->r.expr->kind == F_NUMBER && node->r.expr->v.number == 0)
	    node = node->l.expr;
	else if (node->l.expr->kind == F_NUMBER && node->l.expr->v.number == 0)
	    node = node->r.expr;
    }
    if (node->kind == F_NOT) {
	node = node->r.expr;
	branch = F_BBRANCH_WHEN_ZERO;
    } else {
	branch = F_BBRANCH_WHEN_NON_ZERO;
	if (node->kind == F_NUMBER) {
	    if (node->v.number == 0)
		branch = 0;
	    else 
		branch = F_BBRANCH;
	    node = 0;
	}
    }
    generate(node);
    return branch;
}

#ifdef DEBUG
void
dump_expr_list P2(struct parse_node *, expr, int, indent) {
    if (!expr) return;
    do {
      dump_tree(expr->v.expr, indent);
    } while (expr = expr->r.expr);
}

static void
dump_lvalue_list P2(struct parse_node *, expr, int, indent) {
    int i;

    for (i=0; i<indent; i++) 
        putchar(' ');
    printf("lvalue_list\n");
    while (expr = expr->r.expr)
      dump_tree(expr->l.expr, indent + 2);
}

void
dump_tree P2(struct parse_node *, expr, int, indent) {
    int i;
    if (!expr) return;
    for (i=0; i<indent; i++) 
        putchar(' ');
    switch (expr->kind) {
    case NODE_ASSOC:
	dump_tree(expr->l.expr, indent + 2);
	dump_tree(expr->r.expr, indent + 2);
	break;
    case NODE_CONDITIONAL:
      printf("?\n");
      dump_tree(expr->l.expr, indent + 2);
      for (i=0; i<indent + 2; i++) 
        putchar(' ');
      printf(":\n");
      dump_tree(expr->r.expr->l.expr, indent + 4);
      dump_tree(expr->r.expr->r.expr, indent + 4);
      break;
    case 0:
      printf("<empty>\n");
      dump_tree(expr->l.expr, indent + 2);
      dump_tree(expr->r.expr, indent + 2);
      break;
    case F_NN_RANGE:
    case F_RN_RANGE:
    case F_RR_RANGE:
    case F_NR_RANGE:
    case F_NE_RANGE:
    case F_RE_RANGE:
      printf("%s\n", instrs[expr->kind].name);
      dump_tree(expr->v.expr, indent + 2);
      dump_tree(expr->l.expr, indent + 2);
      dump_tree(expr->r.expr, indent + 2);
      break;
    case F_INDEX_LVALUE:
    case F_INDEX:
    case F_RINDEX:
    case F_RINDEX_LVALUE:
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
      dump_tree(expr->l.expr, indent + 2);
      dump_tree(expr->r.expr, indent + 2);
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
      dump_tree(expr->r.expr, indent + 2);
      break;
    case F_SSCANF:
      printf("sscanf\n");
      dump_tree(expr->l.expr, indent + 2);
      dump_lvalue_list(expr->r.expr, indent + 2);
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
      dump_expr_list(expr->r.expr, indent + 2);
      break;
    case F_AGGREGATE_ASSOC:
      printf("aggregate_assoc\n");
      dump_expr_list(expr->r.expr, indent + 2);
      break;
    case F_CALL_FUNCTION_BY_ADDRESS:
      printf("call function %x\n", expr->v.number);
      dump_expr_list(expr->r.expr, indent + 2);
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
      printf("loop condition: local %i < ", expr->l.expr->v.number);
      if (expr->r.expr->kind == F_NUMBER) {
	printf("%i\n", expr->r.expr->v.number);
      } else {
	printf("local %i\n", expr->r.expr->v.number);
      }
      break;
    default:
      if (expr->kind >= BASE) {
	if (expr->v.number != -1)
	  printf("%s %i\n", instrs[expr->kind].name, expr->v.number);
	else
	  printf("%s\n", instrs[expr->kind].name);
	dump_expr_list(expr->r.expr, indent + 2);
      } else {
	fatal("Unknown node type %d in dump tree.\n", expr->kind);
      }
    }
  }
#endif
