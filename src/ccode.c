/* 
 * code generator for LPC->C compiled code
 */
#include "std.h"

#ifdef LPC_TO_C
#include "lpc_incl.h"
#include "trees.h"
#include "compiler.h"
#include "lex.h"
#include "generate.h"
#include "ccode.h"

#define f_out compilation_output_file

static int current_num_values;
static int switch_type, string_switches, range_switches;
static int catch_number, num_functionals;
static int case_number, case_table_size;

static int foreach_depth = 0;

static int *forward_branch_ptr;
static int forward_branch_stack[100];

static int label;
static int notreached;

static parse_node_t *branch_list[2];

static void c_generate_forward_branch PROT((char));
static void c_update_forward_branch_links PROT((char, parse_node_t *));
static void c_branch_backwards PROT((char, int));
static void c_update_forward_branch PROT((void));
static void c_update_branch_list PROT((parse_node_t *));
static void c_generate_loop PROT((int, parse_node_t *, parse_node_t *,
				  parse_node_t *));
static void c_generate_else PROT((void));

typedef struct switch_table_s {
    struct switch_table_s *next;
    int kind;
    int num_cases;
    int data[1];
} switch_table_t;

static switch_table_t *switch_tables;

static int *add_switch_table P2(int, kind, int, num_cases) {
    switch_table_t *st;
    switch_table_t **stp;

    st = (switch_table_t *)DXALLOC(sizeof(switch_table_t) + (2*num_cases - 1) * sizeof(int), TAG_COMPILER, "switch_table");
    st->next = 0;
    st->kind = kind;
    st->num_cases = num_cases;

    stp = &switch_tables;
    while (*stp)
	stp = &((*stp)->next);
    *stp = st;

    return &st->data[0];
}

static void upd_jump P2(int, addr, int, label) {
    char *p;

    if (!addr) return;
    p = mem_block[current_block].block + addr + 10;

    *p++ = label/100 + '0';
    *p++ = (label/10) % 10 + '0';
    *p++ = label % 10 + '0';
}

static int add_label PROT((void)) {
    if (prog_code + 12 > prog_code_max) {
        mem_block_t *mbp = &mem_block[current_block];

        UPDATE_PROGRAM_SIZE;
        realloc_mem_block(mbp, mbp->current_size * 2);

        prog_code = mbp->block + mbp->current_size;
        prog_code_max = mbp->block + mbp->max_size;
    }
    
    notreached = 0;

    sprintf(prog_code, "label%03i:;\n", label);
    prog_code += 11;
    return label++;
}

static void ins_string P1(char *, s) {
    int l = strlen(s);
    
    if (notreached) return;
    if (prog_code + l + 1 > prog_code_max) {
        mem_block_t *mbp = &mem_block[current_block];

        UPDATE_PROGRAM_SIZE;
        realloc_mem_block(mbp, mbp->current_size * 2);

        prog_code = mbp->block + mbp->current_size;
        prog_code_max = mbp->block + mbp->max_size;
    }
    
    strcpy(prog_code, s);
    prog_code += l;
}

static void ins_vstring P1V(char *, format)
{
    va_list args;
    char buf[1024];
    V_DCL(char *format);

    V_START(args, format);
    V_VAR(char *, format, args);
    vsprintf(buf, format, args);
    va_end(args);

    ins_string(buf);
}

static int ins_jump PROT((void)) {
    int ret = CURRENT_PROGRAM_SIZE;

    if (notreached) return 0;
    ins_string("goto label???;\n");
    notreached = 1;
    return ret;
}

static void
generate_expr_list P1(parse_node_t *, expr) {
    parse_node_t *pn;
    int n, flag;
    
    if (!expr) return;
    pn = expr;
    flag = n = 0;
    do {
	if (pn->type & 1) flag = 1;
	c_generate_node(pn->v.expr);
    } while ((pn = pn->r.expr));
    
    if (flag) {
	pn = expr;
	do {
	    n--;
	    if (pn->type & 1)
		ins_vstring("c_expand_varargs(%i);\n", n);
	} while ((pn = pn->r.expr));
    }
}

static void
generate_lvalue_list P1(parse_node_t *, expr) {
    while ((expr = expr->r.expr)) {
      c_generate_node(expr->l.expr);
      ins_string("c_void_assign();\n");
    }
}

static void
f_quoted_string P2(FILE *, f, char *, s) {
    while (1) {
	switch (*s) {
	case 0: return;
	case '\n':
	    fputs("\\n", f);
	    s++;
	    break;
	case '\t':
	    fputs("\\t", f);
	    s++;
	    break;
	case '\r':
	    fputs("\\r", f);
	    s++;
	    break;
	case '\b':
	    fputs("\\b", f);
	    s++;
	    break;
	case '"':
	    fputs("\\\"", f);
	    s++;
	    break;
	case '\\':
	    fputs("\\\\", f);
	    s++;
	    break;
	default:
	    fputc(*s++, f);
	}
    }
}

void
c_generate_node P1(parse_node_t *, expr) {
    if (!expr) return;

    switch (expr->kind) {
    case NODE_TERNARY_OP:
	c_generate_node(expr->l.expr);
	expr = expr->r.expr;
    case NODE_BINARY_OP:
	c_generate_node(expr->l.expr);
	/* fall through */
    case NODE_UNARY_OP:
	c_generate_node(expr->r.expr);
	/* fall through */
    case NODE_OPCODE:
	ins_string(instrs[expr->v.number].routine);
	break;
    case NODE_TERNARY_OP_1:
	c_generate_node(expr->l.expr);
	expr = expr->r.expr;
	/* fall through */
    case NODE_BINARY_OP_1:
	c_generate_node(expr->l.expr);
	c_generate_node(expr->r.expr);
	ins_vstring(instrs[expr->v.number].routine, expr->type);
	break;
    case NODE_UNARY_OP_1:
	c_generate_node(expr->r.expr);
	/* fall through */
    case NODE_OPCODE_1:
	ins_vstring(instrs[expr->v.number].routine, expr->l.number);
	break;
    case NODE_OPCODE_2:
	ins_vstring(instrs[expr->v.number].routine, expr->l.number, expr->r.number);
	break;
    case NODE_RETURN:
	{
	    int n = foreach_depth;
	    while (n--)
		ins_string("c_exit_foreach();\n");
	    if (expr->r.expr) {
		c_generate_node(expr->r.expr);
		ins_string(instrs[F_RETURN].routine);
	    } else ins_string(instrs[F_RETURN_ZERO].routine);
	    notreached = 1;
	    break;
	}
    case NODE_STRING:
	ins_vstring("C_STRING(%i);\n", expr->v.number);
	break;
    case NODE_REAL:
	ins_vstring("push_real(%f);\n", expr->v.real);
	break;
    case NODE_NUMBER:
	ins_vstring("push_number(%i);\n", expr->v.number);
	break;
    case NODE_LAND_LOR:
	c_generate_node(expr->l.expr);
	c_generate_forward_branch(expr->v.number);
	c_generate_node(expr->r.expr);
	if (expr->l.expr->kind == NODE_BRANCH_LINK) {
	    c_update_forward_branch_links(expr->v.number,expr->l.expr);
	}
	else c_update_forward_branch();
	break;
    case NODE_BRANCH_LINK:
	c_generate_node(expr->l.expr);
	ins_string("BRANCH_LINK(?, ");
	expr->v.number = ins_jump(); 
	prog_code -= 2;
	notreached = 0;
	ins_string(");\n");
	c_generate_node(expr->r.expr);
        break;
    case NODE_CALL_2:
	generate_expr_list(expr->r.expr);
	ins_vstring(instrs[expr->v.number].routine, expr->l.number >> 16,
		    expr->l.number & 0xffff, 
		    (expr->r.expr ? expr->r.expr->kind : 0));
	break;
    case NODE_CALL_1:
	generate_expr_list(expr->r.expr);
	ins_vstring(instrs[expr->v.number].routine, expr->l.number, 
		    (expr->r.expr ? expr->r.expr->kind : 0));
	break;
    case NODE_CALL:
	generate_expr_list(expr->r.expr);
	ins_vstring(instrs[expr->v.number].routine, expr->l.number);
	break;
    case NODE_TWO_VALUES:
	c_generate_node(expr->l.expr);
	c_generate_node(expr->r.expr);
	break;
    case NODE_CONTROL_JUMP:
	{
	    int kind = expr->v.number;
	    
	    if (kind == CJ_BREAK_SWITCH) {
		ins_string("break;\n");
		break;
	    }
	    expr->v.expr = branch_list[kind];
	    expr->l.number = ins_jump();
	    branch_list[kind] = expr;
	    break;
	}
    case NODE_PARAMETER:
	ins_vstring("C_LOCAL(%i);\n", expr->v.number + current_num_values);
	break;
    case NODE_PARAMETER_LVALUE:
	ins_vstring("C_LVALUE(fp + %i);\n",
		    expr->v.number + current_num_values);
	break;
    case NODE_IF:
	if (IS_NODE(expr->v.expr, NODE_UNARY_OP, F_NOT)) {
	    c_generate_node(expr->v.expr->r.expr);
	    c_generate_forward_branch(F_BRANCH_WHEN_NON_ZERO);
	} else {
	    c_generate_node(expr->v.expr);
	    c_generate_forward_branch(F_BRANCH_WHEN_ZERO);
	}
	c_generate_node(expr->l.expr);
	if (expr->r.expr) {
	    c_generate_else();
	    c_generate_node(expr->r.expr);
	}
	c_update_forward_branch();
	break;
    case NODE_LOOP:
	c_generate_loop(expr->type, expr->v.expr, expr->l.expr, expr->r.expr);
	break;
    case NODE_FOREACH:
	{
	    int tmp = 0;

	    c_generate_node(expr->v.expr);
	    if (expr->l.expr->v.number == F_GLOBAL_LVALUE) tmp |= 1;
	    if (expr->r.expr) {
		tmp |= 4;
		if (expr->r.expr->v.number == F_GLOBAL_LVALUE) tmp |= 2;
		ins_vstring("c_foreach(%i, %i, %i);\n", tmp, expr->l.expr->l.number, expr->r.expr->l.number);
	    } else
		ins_vstring("c_foreach(%i, 0, %i);\n", tmp, expr->l.expr->l.number);
	}
	break;
    case NODE_CASE_NUMBER:
	case_table_size++;
	notreached = 0;
	if (switch_type == NODE_SWITCH_RANGES) {
	    ins_vstring("case %i:;\n", case_number);
	    if (expr->v.expr) {
		parse_node_t *other = expr->v.expr;
		case_table_size++;
		expr->v.number = -1;
		other->l.expr = expr->l.expr;
		other->v.number = case_number++;
		expr->l.expr = other;
	    } else {
		expr->v.number = case_number++;
	    }
	} else ins_vstring("case %i:;\n", expr->r.number);
	break;
    case NODE_CASE_STRING:
	notreached = 0;
	case_table_size++;
	expr->v.number = case_number;
	ins_vstring("case %i:;\n", case_number++);
	break;
    case NODE_DEFAULT:
	notreached = 0;
	ins_string("default:;\n");
	break;
    case NODE_SWITCH_STRINGS:
    case NODE_SWITCH_NUMBERS:
    case NODE_SWITCH_RANGES:
    case NODE_SWITCH_DIRECT:
	{
	    int position;
	    parse_node_t *pn;
	    int save_switch_type = switch_type;
	    int save_case_number = case_number;
	    int save_case_table_size = case_table_size;
	    int *p, *stable;

	    switch_type = expr->kind;
	    case_number = 0;
	    case_table_size = 0;
	    position = 0;
	    
	    c_generate_node(expr->l.expr);
	    
	    switch (switch_type) {
	    case NODE_SWITCH_STRINGS:
		ins_vstring("lpc_int = c_string_switch_lookup(sp, string_switch_table_%s_%02i, ", compilation_ident, string_switches++);
		position = prog_code - mem_block[current_block].block;
		ins_string("xxx);\nfree_string_svalue(sp--);\n");
		break;
	    case NODE_SWITCH_DIRECT:
	    case NODE_SWITCH_NUMBERS:
		ins_string("lpc_int = (sp--)->u.number;\n");
		break;
	    case NODE_SWITCH_RANGES:
		ins_vstring("lpc_int = c_range_switch_lookup((sp--)->u.number, range_switch_table_%s_%02i, ", compilation_ident, range_switches++);
		position = prog_code - mem_block[current_block].block;
		ins_string("xxx);\n");
		break;
	    }
	    ins_string("switch (lpc_int) {\n");
	    c_generate_node(expr->r.expr);
	    notreached = 0;
	    ins_string("}\n");

	    if (switch_type == NODE_SWITCH_STRINGS || switch_type == NODE_SWITCH_RANGES) {
		stable = p = add_switch_table(switch_type, case_table_size);
		pn = expr->v.expr;
		while (pn) {
		    if (pn->kind != NODE_DEFAULT) {
			*p++ = pn->r.number;
			*p++ = pn->v.number;
		    }
		    pn = pn->l.expr;
		}
		DEBUG_CHECK(p - stable != case_table_size * 2,
			    "case_table_size was incorrect.\n");
	    }

	    if (position) {
		sprintf(mem_block[current_block].block + position,
			"%3i", case_table_size);
		/* restore the char smashed by the trailing null */
		mem_block[current_block].block[position + 3] = ')'; 
	    }
	    
            switch_type = save_switch_type;
	    case_number = save_case_number;
	    case_table_size = save_case_table_size;
	    break;
	}
    case NODE_CATCH:
	{
	    ins_vstring("{ error_context_t econ%02i;\nc_prepare_catch(&econ%02i);\nif (SETJMP(econ%02i.context)) {\nc_caught_error(&econ%02i);\n} else {\n",
			catch_number, catch_number, catch_number, catch_number);
	    c_generate_node(expr->r.expr);
	    ins_vstring("c_end_catch(&econ%02i);\n}\n}\n", catch_number++);
	    break;
	}
    case NODE_TIME_EXPRESSION:
	{
	    ins_string("{ int start_sec,start_usec,end_sec,end_usec;\nget_usec_clock(&start_sec,&start_usec);\n");
	    c_generate_node(expr->r.expr);
	    ins_string("get_usec_clock(&end_sec,&end_usec);\nend_usec=(end_sec - start_sec) * 1000000 + end_usec - start_usec;\npush_number(end_usec);\n}\n");
	    break;
	}
    case NODE_LVALUE_EFUN:
	c_generate_node(expr->l.expr);
	generate_lvalue_list(expr->r.expr);
	break;
    case NODE_FUNCTION_CONSTRUCTOR:
	if (expr->r.expr) {
	    generate_expr_list(expr->r.expr);
	    ins_vstring("C_AGGREGATE(%i);\n", expr->r.expr->kind);
	} else 
	    ins_string("push_number(0);\n");
	
	switch (expr->v.number & 0xff) {
	case FP_SIMUL:
	case FP_LOCAL:
	    ins_vstring("c_function_constructor(%i, %i);\n", 
			expr->v.number & 0xff, expr->v.number >> 8);
	    break;
	case FP_EFUN:
	    ins_vstring("c_function_constructor(%i, %i);\n", 
		    expr->v.number & 0xff, predefs[expr->v.number >> 8].token);
	    break;
	case FP_FUNCTIONAL:
	case FP_FUNCTIONAL | FP_NOT_BINDABLE:
	    {
		int save_current_num_values = current_num_values;
		int save_current_block = current_block;
		
		ins_vstring("c_functional(%i, %i, (POINTER_INT)LPCFUNCTIONAL_%03i);\n", 
			    expr->v.number & 0xff, expr->v.number >> 8, num_functionals);
		current_num_values = expr->r.expr ? expr->r.expr->kind : 0;
		switch_to_block(A_FUNCTIONALS);
		ins_vstring("static void LPCFUNCTIONAL_%03i PROT((void)) {\n", num_functionals++);
		c_generate_node(expr->l.expr);
		ins_vstring("c_return();\n}\n\n");
		switch_to_block(save_current_block);
		current_num_values = save_current_num_values;
		break;
	    }
	}
	break;
    case NODE_ANON_FUNC:
	{
	    int save_fd = foreach_depth;
	    int save_current_block = current_block;
	    int save_notreached = notreached;
	    
	    foreach_depth = 0;
	    ins_vstring("c_anonymous(%i, %i, (POINTER_INT)LPCFUNCTIONAL_%03i);\n",
			expr->v.number, expr->l.number, num_functionals);
	    switch_to_block(A_FUNCTIONALS);
	    ins_vstring("void LPCFUNCTIONAL_%03i PROT((void)) {\n", num_functionals++);
	    c_generate_node(expr->r.expr);
	    notreached = 0;
	    ins_string("\n}\n\n");
	    switch_to_block(save_current_block);
	    foreach_depth = save_fd;
	    notreached = save_notreached;
	    break;
	}
    case NODE_EFUN:
	{
	    parse_node_t *node = expr->r.expr;
	    int num_arg = expr->l.number;
	    int novalue_used = expr->v.number & NOVALUE_USED_FLAG;
	    int f = expr->v.number & ~NOVALUE_USED_FLAG;
	    int idx = 1;

	    generate_expr_list(node);
	    while (node) {
		if (idx == 5) break;
		ins_vstring("CHECK_TYPES(sp - %i, %i, %i, %i);\n",
			    num_arg - idx, instrs[f].type[idx - 1], 
			    idx, f);
		idx++;
		node = node->r.expr;
	    }
	    if (instrs[f].max_arg == -1) {
		ins_vstring("st_num_arg = %i + num_varargs;\nnum_varargs = 0;\n", num_arg);
	    } else {
		ins_vstring("st_num_arg = %i;\n", num_arg);
	    }
	    ins_vstring("f_%s();\n", instrs[f].name);
	    if (novalue_used) {
		/* the value of a void efun was used.  Put in a zero. */
		ins_string("push_number(0);\n");
	    }
	}
	break;
    default:
	    fatal("Unknown node %i in c_generate_node.\n", expr->kind);
    }
}

static void c_generate_loop P4(int, test_first, parse_node_t *, block,
			       parse_node_t *, inc, parse_node_t *, test) {
    parse_node_t *save_breaks = branch_list[CJ_BREAK];
    parse_node_t *save_continues = branch_list[CJ_CONTINUE];
    int forever = node_always_true(test);
    int pos;

    if (test_first == 2) foreach_depth++;
    branch_list[CJ_BREAK] = branch_list[CJ_CONTINUE] = 0;
    if (!forever && test_first)
	c_generate_forward_branch(F_BRANCH);
    pos = add_label();
    c_generate_node(block);
    c_update_branch_list(branch_list[CJ_CONTINUE]);
    if (inc) c_generate_node(inc);
    if (!forever && test_first)
	c_update_forward_branch();
    if (test->kind == F_LOOP_COND_LOCAL || test->kind == F_LOOP_COND_NUMBER) {
	c_generate_node(test);
	ins_vstring("goto label%03i;\n", pos);
	notreached = 1;
    } else {
	if (test_first == 2)
	    ins_vstring("if (c_next_foreach())\ngoto label%03i;\n", pos);
	else
	    c_branch_backwards(generate_conditional_branch(test), pos);
    }
    c_update_branch_list(branch_list[CJ_BREAK]);
    branch_list[CJ_BREAK] = save_breaks;
    branch_list[CJ_CONTINUE] = save_continues;
    if (test_first == 2) foreach_depth--;
}

void
c_generate_inherited_init_call P2(int, index, short, f) {
    ins_vstring("c_call_inherited(%i, %i, 0);\npop_stack();\n", index, (int)f);
}

void c_start_function P1(char *, fname) {
    notreached = 0;
    ins_vstring("static void LPC_%s__%s() {\n", compilation_ident, fname);
}

void c_end_function() {
    notreached = 0;
    ins_string("}\n\n");
}

void c_generate___INIT() {
}

static void c_generate_forward_branch P1(char, b) {
    switch (b) {
    case F_LAND:
	ins_string("C_LAND(");
	break;
    case F_LOR:
	ins_string("C_LOR(");
	break;
    case F_BRANCH_WHEN_NON_ZERO:
	ins_string("C_BRANCH_WHEN_NON_ZERO(");
	break;
    case F_BRANCH_WHEN_ZERO:
	ins_string("C_BRANCH_WHEN_ZERO(");
	break;
    case F_BRANCH:
	*forward_branch_ptr++ = ins_jump();
    case 0:
	return;
	break;
    default:
	fatal("Unknown opcode %i in generate_forward_branch\n", (int)b);
    }
    *forward_branch_ptr++ = ins_jump();
    prog_code -= 2;
    notreached = 0;
    ins_string(");\n");
}

static void
c_update_forward_branch() {
    int i = *--forward_branch_ptr;

    upd_jump(i, label);
    add_label();
}

static void c_update_forward_branch_links P2(char, kind, parse_node_t *, link_start){
    int i = *--forward_branch_ptr;
    int our_label;
    char *p;

    upd_jump(i, our_label = add_label());
    do {
	i = link_start->v.number;
	p = mem_block[current_block].block + i - 3;
	if (kind == F_LOR)
	    *p = '!';
	else
	    *p = ' ';
	
	upd_jump(i, our_label);
	link_start = link_start->l.expr;
    } while (link_start->kind == NODE_BRANCH_LINK);
}

static void
c_branch_backwards P2(char, b, int, addr) {
    switch (b) {
    case F_BBRANCH_WHEN_ZERO:
	ins_vstring("C_BRANCH_WHEN_ZERO(goto label%03i);\n", addr);
	break;
    case F_BBRANCH_WHEN_NON_ZERO:
	ins_vstring("C_BRANCH_WHEN_NON_ZERO(goto label%03i);\n", addr);
	break;
    case F_BBRANCH:
	ins_vstring("goto label%03i;\n", addr);
	notreached = 1;
	break;
    case F_BBRANCH_LT:
	ins_vstring("C_BBRANCH_LT(goto label%03i);\n", addr);
	break;
    case F_WHILE_DEC:
	ins_vstring("goto label%03i;\n", addr);
    }
}

static void
c_update_branch_list P1(parse_node_t *, bl) {
    if (bl) {
	do {
	    upd_jump(bl->l.number, label);
	} while ((bl = bl->v.expr));
	add_label();
    }
}

static void
c_generate_else() {
    int tmp = ins_jump();

    upd_jump(forward_branch_ptr[-1], label);
    add_label();
    forward_branch_ptr[-1] = tmp;
}

void
c_initialize_parser() {
    branch_list[CJ_BREAK] = 0;
    branch_list[CJ_CONTINUE] = 0;
    forward_branch_ptr = &forward_branch_stack[0];
    foreach_depth = 0;
    
    current_block = A_PROGRAM;
    prog_code = mem_block[A_PROGRAM].block;
    prog_code_max = mem_block[A_PROGRAM].block + mem_block[A_PROGRAM].max_size;

    num_functionals = catch_number = string_switches = range_switches = 0;
    switch_tables = 0;
    label = 0;
    notreached = 0;
}

#if 0
static char *protect_allocated_string = 0;

static char *
protect P1(char *, str) {
    static char buf[1024];
    char *p;
    int size = 0;

    if (protect_allocated_string)
	FREE(protect_allocated_string);

    p = str;
    while (*p) {
	if (*p == '"' || *p == '\\') size += 2;
	else size++;
	p++;
    }

    if (size < 1024) {
	p = buf;
	while (*str) {
	    if (*str == '"' || *str == '\\') *p++ = '\\';
	    *p++ = *str++;
	}
	*p = 0;
	return buf;
    } else {
	p = protect_allocated_string = DXALLOC(size + 1, TAG_STRING, "protect");
	while (*str) {
	    if (*str == '"' || *str == '\\') *p++ = '\\';
	    *p++ = *str++;
	}
	*p++ = 0;
	return protect_allocated_string;
    }	
}
#endif

void
c_generate_final_program P1(int, x) {
    switch_table_t *st, *next;
    int i;
    int index = 0;
    compiler_function_t *funp;
    
    if (!x) {
	if (string_switches) {
	    st = switch_tables;
	    while (st) {
		if (st->kind == NODE_SWITCH_STRINGS) {
		    fprintf(f_out, "static string_switch_entry_t string_switch_table_%s_%02i[] = {\n", compilation_ident, index++);
		    for (i = 0; i < st->num_cases; i++) {
			fprintf(f_out, "{\"");
			f_quoted_string(f_out, PROG_STRING(st->data[i*2]));
			fprintf(f_out, "\", %i },\n", st->data[i*2+1]);
		    }
		    fprintf(f_out, "{ 0, 0 }\n};\n\n");
		}
		st = st->next;
	    }
	    fprintf(f_out, "static string_switch_entry_t *string_switch_tables[] = {\n");
	    for (i = 0; i < index; i++)
		fprintf(f_out, "string_switch_table_%s_%02i,\n", compilation_ident, i);
	    fprintf(f_out, "0\n};\n\n");
	}


	if (range_switches) {
	    index = 0;
	    st = switch_tables;
	    while (st) {
		if (st->kind == NODE_SWITCH_RANGES) {
		    fprintf(f_out, "static range_switch_entry_t range_switch_table_%s_%02i[] = {\n", compilation_ident, index++);
		    for (i = 0; i < st->num_cases; i++) {
			fprintf(f_out, "{ %i, %i },\n", 
				st->data[i*2], st->data[i*2+1]);
		    }
		    fprintf(f_out, "{ 0, -2 }\n};\n\n");
		}
		st = st->next;
	    }
	    fprintf(f_out, "static range_switch_entry_t *range_switch_tables_%s[] = {\n", compilation_ident);
	    for (i = 0; i < index; i++)
		fprintf(f_out, "range_switch_table_%s_%02i,\n", compilation_ident, i);
	    fprintf(f_out, "};\n\n");
	}

	st = switch_tables;
	while (st) {
	    next = st->next;
	    FREE((char *)st);
	    st = next;
	}

	fwrite(mem_block[A_FUNCTIONALS].block,
	       mem_block[A_FUNCTIONALS].current_size, 1, f_out);

	if (mem_block[A_INITIALIZER].current_size) {
	    fprintf(f_out, "static void LPCINIT_%s() {\n", compilation_ident);
	    fwrite(mem_block[A_INITIALIZER].block, 
		   mem_block[A_INITIALIZER].current_size, 1, f_out);
	    fprintf(f_out, "}\n");
	}
	
	fwrite(mem_block[A_PROGRAM].block,
	       mem_block[A_PROGRAM].current_size, 1, f_out);

	current_block = A_PROGRAM;
	prog_code = mem_block[A_PROGRAM].block;

	fprintf(f_out, "\n\nstatic void (*functions[])() = {\n");
	for (i = 0; i < mem_block[A_COMPILER_FUNCTIONS].current_size/sizeof(*funp); i++) {
	    funp = COMPILER_FUNC(i);
	    if (!(FUNCTION_FLAGS(funp->runtime_index) & NAME_NO_CODE)) {
		if (funp->name[0] == APPLY___INIT_SPECIAL_CHAR)
		    fprintf(f_out, "LPCINIT_%s,\n", compilation_ident);
		else
		    fprintf(f_out, "LPC_%s__%s,\n", compilation_ident,
			    funp->name);
	    }
	}
	fprintf(f_out, "0\n};\n");
	{
	    char buf[1024];
	    int l;

	    strcpy(buf, current_file);
	    l = strlen(current_file);
	    if (buf[l-1] == 'c' && buf[l-2] == '.')
		buf[l-2] = 0;
	    fprintf(f_out, "\ninterface_t LPCINFO_%s = {\n    \"%s\",\n", 
		    compilation_ident, buf);
	}
	fprintf(f_out, "    functions,\n");
	if (string_switches)
	    fprintf(f_out, "    string_switch_tables\n};\n");
	else
	    fprintf(f_out, "    0\n};\n");
    }
}

void c_analyze P1(parse_node_t *, node) {
    /* future work */
}
#endif
