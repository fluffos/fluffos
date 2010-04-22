#define SUPPRESS_COMPILER_INLINES
#include "std.h"
#include "generate.h"
#include "compiler.h"

static parse_node_t *optimize (parse_node_t *);
static parse_node_t **last_local_refs = 0;
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

static void
optimize_expr_list (parse_node_t * expr) {
    if (!expr) return;
    do {
	expr->v.expr = optimize(expr->v.expr);
    } while ((expr = expr->r.expr));
}

static void
optimize_lvalue_list (parse_node_t * expr) {
    while ((expr = expr->r.expr)) {
	expr->v.expr = optimize(expr->l.expr);
    }
}

#define OPT(x) x = optimize(x)
#define OPTIMIZER_IN_LOOP        1
#define OPTIMIZER_IN_COND	 2 /* switch or if or ?: */
static int optimizer_state = 0;

static parse_node_t *
optimize (parse_node_t * expr) {
    if (!expr) return 0;

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
			last_local_refs[x] = 0;
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
		last_local_refs[expr->l.number] = 0;
	    }
	}
	break;
    case NODE_OPCODE_1:
	if (expr->v.number == F_LOCAL || expr->v.number == F_LOCAL_LVALUE) {
	    if (expr->v.number == F_LOCAL) {
		if(!optimizer_state) {
		    last_local_refs[expr->l.number] = expr;
		    break;
		}
	    }
	    last_local_refs[expr->l.number] = 0;
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
    case NODE_BRANCH_LINK:
      {
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
    case NODE_TWO_VALUES:
	OPT(expr->l.expr);
	OPT(expr->r.expr);
	break;
    case NODE_CONTROL_JUMP:
    case NODE_PARAMETER:
    case NODE_PARAMETER_LVALUE:
	break;
    case NODE_IF:
	{
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
    case NODE_LOOP:
	{
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
    case NODE_SWITCH_RANGES:
	{
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
	if (expr->r.expr)
	    optimize_expr_list(expr->r.expr); /* arguments */
	break;
    case NODE_ANON_FUNC:
	break;
    case NODE_EFUN:
	optimize_expr_list(expr->r.expr);
	break;
    default:
	break;
    }
    return expr;
}

#ifdef DEBUG
char *lpc_tree_name[] = {
    "return", "two values", "opcode", "opcode_1", "opcode_2",
    "unary op", "unary op_1", "binary op", "binary op_1",
    "ternary op", "ternary op_1", "control jump", "loop", "call",
    "call_1", "call_2", "&& ||", "foreach", "lvalue_efun", "switch_range",
    "switch_string", "switch_direct", "switch_number", "case_number",
    "case_string", "default", "if", "branch link", "parameter",
    "parameter_lvalue", "efun", "anon func", "real", "number",
    "string", "function", "catch"
};

static void lpc_tree (parse_node_t * dest, int num) {
    parse_node_t *pn;
    
    dest->kind = NODE_CALL;
    dest->v.number = F_AGGREGATE;
    dest->type = TYPE_ANY | TYPE_MOD_ARRAY;
    dest->l.number = num;
    if (!num)
	dest->r.expr = 0;
    else {
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

static void lpc_tree_number (parse_node_t * dest, int num) {
    CREATE_NUMBER(dest->v.expr, num);
}

static void lpc_tree_real (parse_node_t * dest, float real) {
    CREATE_REAL(dest->v.expr, real);
}

static void lpc_tree_expr (parse_node_t * dest, parse_node_t * expr) {
    dest->v.expr = new_node_no_line();
    lpc_tree_form(expr, dest->v.expr);
}

static void lpc_tree_string (parse_node_t * dest, const char * str) {
    CREATE_STRING(dest->v.expr, str);
}

static void lpc_tree_list (parse_node_t * dest, parse_node_t * expr) {
    parse_node_t *pn;
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

void 
lpc_tree_form (parse_node_t * expr, parse_node_t * dest) {
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
	lpc_tree(dest,1);
	lpc_tree_string(ARG_1, "!GARBAGE!");
	return;
    }
    lpc_tree_string(ARG_1, lpc_tree_name[expr->kind]);
}
#endif

short
generate (parse_node_t * node) {
    short where = CURRENT_PROGRAM_SIZE;

    if (num_parse_error) return 0;
    {
	i_generate_node(node);
    }
    free_tree();
    
    return where;
}

static void optimizer_start_function (int n) {
    if (n) {
	last_local_refs = CALLOCATE(n, parse_node_t *, TAG_COMPILER, "c_start_function");
	optimizer_num_locals = n;
	while (n--) {
	    last_local_refs[n] = 0;
	}
    } else last_local_refs = 0;
}

static void optimizer_end_function (void) {
    int i;
    if (last_local_refs) {
	for (i = 0; i < optimizer_num_locals; i++) 
	    if (last_local_refs[i]) {
		last_local_refs[i]->v.number = F_TRANSFER_LOCAL;
	    }
	FREE(last_local_refs);
	last_local_refs = 0;
    }
}

short generate_function (function_t * f, parse_node_t * node, int num) {
    short ret;
    if (pragmas & PRAGMA_OPTIMIZE) {
	optimizer_start_function(num);
	optimizer_state = 0;
	node = optimize(node);
	optimizer_end_function();
    }
    ret = generate(node);
    return ret;
}

int
node_always_true (parse_node_t * node) {
    if (!node) return 1;
    if (node->kind == NODE_NUMBER)
	return node->v.number;
    return 0;
}

int
generate_conditional_branch (parse_node_t * node) {
    int branch;

    if (!node)
	return F_BBRANCH;

    /* only have to handle while (x != 0) since while (x == 0) will be
     * handled by the x == 0 -> !x and !x optimizations.
     */
    if (IS_NODE(node, NODE_BINARY_OP, F_NE)) {
	if (IS_NODE(node->r.expr, NODE_NUMBER, 0))
	    node = node->l.expr;
	else if (IS_NODE(node->l.expr, NODE_NUMBER, 0))
	    node = node->r.expr;
    }
    if (IS_NODE(node, NODE_UNARY_OP, F_NOT)) {
	node = node->r.expr;
	branch = F_BBRANCH_WHEN_ZERO;
    } else {
	branch = F_BBRANCH_WHEN_NON_ZERO;
	if (node->kind == NODE_NUMBER) {
	    if (node->v.number == 0)
		branch = 0;
	    else 
		branch = F_BBRANCH;
	    node = 0;
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

#ifdef DEBUG
void
dump_expr_list (parse_node_t * expr) {
    if (!expr) return;
    do {
      dump_tree(expr->v.expr);
    } while ((expr = expr->r.expr));
}

static void
dump_lvalue_list (parse_node_t * expr) {
    printf("(lvalue_list ");
    while ((expr = expr->r.expr))
      dump_tree(expr->l.expr);
}

void
dump_tree (parse_node_t * expr) {
    if (!expr) return;

    switch (expr->kind) {
    case NODE_TERNARY_OP:
	printf("(%s ", instrs[expr->r.expr->v.number].name);
	dump_tree(expr->l.expr);
	expr = expr->r.expr;
	dump_tree(expr->l.expr);
	dump_tree(expr->r.expr);
	printf(")");
	break;
    case NODE_BINARY_OP:
	printf("(%s ", instrs[expr->v.number].name);
	dump_tree(expr->l.expr);
	dump_tree(expr->r.expr);
	printf(")");
	break;
    case NODE_UNARY_OP:
	printf("(%s ", instrs[expr->v.number].name);
	dump_tree(expr->r.expr);
	printf(")");
	break;
    case NODE_OPCODE:
	printf("(%s)", instrs[expr->v.number].name);
	break;
    case NODE_TERNARY_OP_1:
	{
	    int p = expr->type;
	    printf("(%s ", instrs[expr->r.expr->v.number].name);
	    dump_tree(expr->l.expr);
	    expr = expr->r.expr;
	    dump_tree(expr->l.expr);
	    dump_tree(expr->r.expr);
	    printf(" %i)", p);
	    break;
	}
    case NODE_BINARY_OP_1:
	printf("(%s ", instrs[expr->v.number].name);
	dump_tree(expr->l.expr);
	dump_tree(expr->r.expr);
	printf(" %i)", expr->type);
	break;
    case NODE_UNARY_OP_1:
	printf("(%s ", instrs[expr->v.number].name);
	dump_tree(expr->r.expr);
	printf(" %i)", expr->l.number);
	break;
    case NODE_OPCODE_1:
	printf("(%s %i)", instrs[expr->v.number].name, expr->l.number);
	break;
    case NODE_OPCODE_2:
	printf("(%s %i %i)", instrs[expr->v.number].name, expr->l.number, expr->r.number);
	break;
    case NODE_RETURN:
	if (expr->r.expr) {
	    printf("(return ");
	    dump_tree(expr->r.expr);
	    printf(")");
	} else {
	    printf("(return_zero)");
	}
	break;
    case NODE_STRING:
	printf("(string %i)", expr->v.number);
	break;
    case NODE_REAL:
	printf("(real %f)", expr->v.real);
	break;
    case NODE_NUMBER:
	printf("(number %i)", expr->v.number);
	break;
    case NODE_LAND_LOR:
	if (expr->v.number == F_LAND)
	    printf("(&& ");
	else
	    printf("(|| ");
	dump_tree(expr->l.expr);
	dump_tree(expr->r.expr);
	printf(")");
	break;
    case NODE_BRANCH_LINK:
	printf("(branch_link ");
	dump_tree(expr->l.expr);
	dump_tree(expr->r.expr);
	printf(")");
	break;
    case NODE_CALL_2:
	printf("(%s %i %i %i ", instrs[expr->v.number].name, expr->l.number >> 16,
	       expr->l.number & 0xffff, (expr->r.expr ? expr->r.expr->kind : 0));
	dump_expr_list(expr->r.expr);
	printf(")");
	break;
    case NODE_CALL_1:
	printf("(%s %i %i ", instrs[expr->v.number].name, expr->l.number,
	       (expr->r.expr ? expr->r.expr->kind : 0));
	dump_expr_list(expr->r.expr);
	printf(")");
	break;
    case NODE_CALL:
	printf("(%s %i ", instrs[expr->v.number].name, expr->l.number);
	dump_expr_list(expr->r.expr);
	printf(")");
	break;
    case NODE_TWO_VALUES:
	dump_tree(expr->l.expr);
	printf("\n");
	dump_tree(expr->r.expr);
	break;
    case NODE_CONTROL_JUMP:
	if (expr->v.number == CJ_BREAK_SWITCH) {
	    printf("(break_switch)");
	} else if (expr->v.number == CJ_BREAK) {
	    printf("(break)");
	} else if (expr->v.number == CJ_CONTINUE) {
	    printf("(continue)");
	} else {
	    printf("(UNKNOWN CONTROL JUMP)");
	}
	break;
    case NODE_PARAMETER:
	printf("(parameter %i)", expr->v.number);
	break;
    case NODE_PARAMETER_LVALUE:
	printf("(parameter_lvalue %i)", expr->v.number);
	break;
    case NODE_IF:
	printf("(if ");
	dump_tree(expr->v.expr);
	printf("\n");
	dump_tree(expr->l.expr);
	if (expr->r.expr) {
	    printf("\n");
	    dump_tree(expr->r.expr);
	}
	printf(")\n");
	break;
    case NODE_LOOP:
	printf("(loop %i\n", expr->type);
	dump_tree(expr->v.expr);
	printf("\n");
	dump_tree(expr->l.expr);
	printf("\n");
	dump_tree(expr->r.expr);
	printf(")\n");
	break;
    case NODE_FOREACH:
	printf("(foreach ");
	dump_tree(expr->l.expr);
	dump_tree(expr->r.expr);
	dump_tree(expr->v.expr);
	printf(")\n");
	break;
    case NODE_CASE_NUMBER:
    case NODE_CASE_STRING:
	printf("(case)");
	break;
    case NODE_DEFAULT:
	printf("(default)");
	break;
    case NODE_SWITCH_STRINGS:
    case NODE_SWITCH_NUMBERS:
    case NODE_SWITCH_DIRECT:
    case NODE_SWITCH_RANGES:
	printf("(switch ");
	dump_tree(expr->l.expr);
	dump_tree(expr->r.expr);
	printf(")");
	break;
    case NODE_CATCH:
	printf("(catch ");
	dump_tree(expr->r.expr);
	printf(")");
	break;
    case NODE_LVALUE_EFUN:
	printf("(lvalue_efun ");
	dump_tree(expr->l.expr);
	dump_lvalue_list(expr->r.expr);
	printf(")");
	break;
    case NODE_FUNCTION_CONSTRUCTOR:
	printf("(function %i ", expr->v.number & 0xff);
	if (expr->r.expr) {
	    printf("(array ");
	    dump_expr_list(expr->r.expr);
	    printf(")");
	} else {
	    printf("(number 0)");
	}
	switch (expr->v.number & 0xff) {
	case FP_SIMUL:
	    printf("(fp-simul %i)", expr->v.number >> 8);
	    break;
	case FP_LOCAL:
	    printf("(fp-local %i)", expr->v.number >> 8);
	    break;
	case FP_EFUN:
	    printf("(fp-efun %s)", instrs[expr->v.number >> 8].name);
	    break;
	case FP_FUNCTIONAL:
	case FP_FUNCTIONAL | FP_NOT_BINDABLE:
	    printf("(fp-functional %i ", expr->v.number >> 8);
	    dump_tree(expr->l.expr);
	    printf(")");
	    break;
	}
	printf(" %i)", expr->v.number >> 8);
	break;
    case NODE_ANON_FUNC:
	printf("(anon-func %i %i ", expr->v.number, expr->l.number);
	dump_tree(expr->r.expr);
	printf(")");
	break;
    case NODE_EFUN:
	printf("(%s ", instrs[expr->v.number & ~NOVALUE_USED_FLAG].name);
	dump_expr_list(expr->r.expr);
	printf(")");
	break;
    default:
	printf("(unknown)");
	break;
    }
    fflush(stdout);
}
#endif
