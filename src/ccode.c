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
static int case_number, case_table_size;

static int current_frame_size;

static int *forward_branch_ptr;
static int forward_branch_stack[100];

static int label;
static int notreached;

static parse_node_t break_dummy = { NODE_BREAK, 0, 1 };
static parse_node_t cont_dummy = { NODE_CONTINUE, 0, 1 };

static parse_node_t *break_ptr;
static parse_node_t *cont_ptr;

static void c_restore_loop_info();
static void c_save_loop_info();
static void c_generate_forward_branch PROT((char));
static void c_update_forward_branch_links PROT((char, parse_node_t *));
static void c_branch_backwards PROT((char, int));
static void c_update_forward_branch();
static void c_update_breaks();
static void c_update_continues();
static void c_generate_else();

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

static void
generate_expr_list P1(parse_node_t *, expr) {
    if (!expr) return;
    do {
      c_generate_node(expr->v.expr);
    } while (expr = expr->r.expr);
}

static void upd_jump P2(int, addr, int, label) {
    char *p;

    if (!addr) return;
    p = mem_block[current_block].block + addr + 10;

    *p++ = label/100 + '0';
    *p++ = (label/10) % 10 + '0';
    *p++ = label % 10 + '0';
}

static int add_label() {
    if (prog_code + 11 > prog_code_max) {
        mem_block_t *mbp = &mem_block[current_block];

        UPDATE_PROGRAM_SIZE;
        realloc_mem_block(mbp, mbp->current_size * 2);

        prog_code = mbp->block + mbp->current_size;
        prog_code_max = mbp->block + mbp->max_size;
    }
    
    notreached = 0;

    sprintf(prog_code, "label%03i:\n", label);
    prog_code += 10;
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

static void ins_vstring PVARGS(va_alist)
{
    va_list args;
    char buf[1024];
    char *format;

#ifdef HAS_STDARG_H
    va_start(args, va_alist);
    format = va_alist;
#else
    va_start(args);
    format = va_arg(args, char *);
#endif
    vsprintf(buf, format, args);
    va_end(args);

    ins_string(buf);
}

static int ins_jump() {
    int ret = CURRENT_PROGRAM_SIZE;

    if (notreached) return 0;
    ins_string("goto label???;\n");
    notreached = 1;
    return ret;
}

static void
generate_lvalue_list P1(parse_node_t *, expr) {
    while (expr = expr->r.expr) {
      c_generate_node(expr->l.expr);
      ins_string("c_void_assign();\n");
    }
}

static void
ins_quoted_string P1(char *, s) {
    if (notreached) return;
    while (1) {
	while (prog_code + 1 < prog_code_max) {
	    switch (*s) {
	    case 0: return;
	    case '\n':
		*prog_code++ = '\\';
		*prog_code++ = 'n';
		s++;
		break;
	    case '\t':
		*prog_code++ = '\\';
		*prog_code++ = 't';
		s++;
		break;
	    case '\r':
		*prog_code++ = '\\';
		*prog_code++ = 'r';
		s++;
		break;
	    case '\b':
		*prog_code++ = '\\';
		*prog_code++ = 'b';
		s++;
		break;
	    case '"':
		*prog_code++ = '\\';
		*prog_code++ = '"';
		s++;
		break;
	    case '\\':
		*prog_code++ = '\\';
		*prog_code++ = '\\';
		s++;
		break;
	    default:
		*prog_code++ = *s++;
	    }
	}
	{
	    mem_block_t *mbp = &mem_block[current_block];
	    
	    UPDATE_PROGRAM_SIZE;
	    realloc_mem_block(mbp, mbp->current_size * 2);
	    
	    prog_code = mbp->block + mbp->current_size;
	    prog_code_max = mbp->block + mbp->max_size;
	}
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
	c_generate_node(expr->l.expr);
	/* fall through */
    case F_PRE_INC:
    case F_PRE_DEC:
    case F_INC:
    case F_DEC:
    case F_POST_INC:
    case F_POST_DEC:
    case F_NOT:
    case F_COMPL:
    case F_NEGATE:
    case F_POP_VALUE:
	c_generate_node(expr->r.expr);
	/* fall through */
#ifdef DEBUG
    case F_BREAK_POINT:
#endif
    case F_BREAK:
    case F_CONST0:
    case F_CONST1:
	ins_string(instrs[expr->kind].routine);
	break;
    case F_RETURN_ZERO:
	if (pragmas & PRAGMA_EFUN) {
	    if (current_frame_size)
		ins_vstring("pop_n_elems(%i);\n\n", current_frame_size);
	    ins_string("push_number(0);\n");
	} else
	    ins_string("c_return_zero();\n");
	ins_string("return;\n");
	notreached = 1;
	break;
    case F_RETURN:
	c_generate_node(expr->r.expr);
	if (pragmas & PRAGMA_EFUN) {
	    if (current_frame_size)
		ins_vstring("c_efun_return(%i);\n", current_frame_size);
	} else
	    ins_string("c_return();\n");
	ins_string("return;\n");
	notreached = 1;
	break;
    case F_VOID_ASSIGN:
	if (expr->l.expr->kind == F_LOCAL_LVALUE) {
	    c_generate_node(expr->r.expr);
	    ins_vstring("c_void_assign_local(fp + %i);\n", 
		       expr->l.expr->v.number);
	    break;
	}
    case F_ASSIGN: /* note these are backwards */
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
    case F_VOID_ADD_EQ:
	c_generate_node(expr->r.expr);
	c_generate_node(expr->l.expr);
	ins_string(instrs[expr->kind].routine);
	break;
    case F_NN_RANGE:
    case F_RN_RANGE:
    case F_RR_RANGE:
    case F_NR_RANGE:
    case F_NN_RANGE_LVALUE:
    case F_RN_RANGE_LVALUE:
    case F_NR_RANGE_LVALUE:
    case F_RR_RANGE_LVALUE:
	c_generate_node(expr->l.expr);
	c_generate_node(expr->r.expr);
	c_generate_node(expr->v.expr);
	ins_string(instrs[expr->kind].routine);
	break;
    case NODE_NE_RANGE_LVALUE:
        c_generate_node(expr->l.expr);
	ins_string("push_number(1);\n");
        c_generate_node(expr->v.expr);
	ins_string("f_nr_range_lvalue();\n");
        break;
    case NODE_RE_RANGE_LVALUE:
        c_generate_node(expr->l.expr);
	ins_string("push_number(1);\n");
        c_generate_node(expr->v.expr);
	ins_string("f_rr_range_lvalue();\n");
        break;
    case F_RE_RANGE:
    case F_NE_RANGE:
        c_generate_node(expr->l.expr);
        c_generate_node(expr->v.expr);
	ins_string(instrs[expr->kind].routine);
        break;
    case F_STRING:
	if (pragmas & PRAGMA_EFUN) {
	    ins_string("push_constant_string(\"");
	    ins_quoted_string(PROG_STRING(expr->v.number));
	    ins_string("\");\n");
	} else 
	    ins_vstring("C_STRING(%i);\n", expr->v.number);
	break;
    case F_REAL:
	ins_vstring("push_real(%f);\n", expr->v.real);
	break;
    case F_MEMBER:
    case F_MEMBER_LVALUE:
	c_generate_node(expr->r.expr);
	ins_vstring("c_member(%i);\n", expr->v.number);
	break;
    case F_NEW_CLASS:
	ins_vstring("c_new_class(%i);\n", expr->v.number);
	break;
    case F_NBYTE:
    case F_BYTE:
    case F_NUMBER:
	ins_vstring("push_number(%i);\n", expr->v.number);
	break;
    case F_LOR:
    case F_LAND:
	c_generate_node(expr->l.expr);
	c_generate_forward_branch(expr->kind);
	c_generate_node(expr->r.expr);
	if (expr->l.expr->kind == NODE_BRANCH_LINK) {
	    c_update_forward_branch_links(expr->kind,expr->l.expr);
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
    case F_AGGREGATE:
	generate_expr_list(expr->r.expr);
	ins_vstring("C_AGGREGATE(%i);\n", expr->v.number);
	break;
    case F_AGGREGATE_ASSOC:
	generate_expr_list(expr->r.expr);
	ins_vstring("C_AGGREGATE_ASSOC(%i);\n", expr->v.number);
	break;
    case NODE_COMMA:
    case NODE_ASSOC:
	c_generate_node(expr->l.expr);
	c_generate_node(expr->r.expr);
	break;
    case NODE_BREAK:
	expr->r.expr = break_ptr;
	expr->type = 0;
	expr->v.number = ins_jump();
	break_ptr = expr;
	break;
    case NODE_CONTINUE:
	expr->r.expr = cont_ptr;
	expr->v.number = ins_jump();
	expr->type = 0;
	cont_ptr = expr;
	break;
    case NODE_STATEMENTS:
	c_generate_node(expr->l.expr);
	c_generate_node(expr->r.expr);
	break;
    case NODE_PARAMETER:
	ins_vstring("C_LOCAL(%i);\n", expr->v.number + current_num_values);
	break;
    case NODE_PARAMETER_LVALUE:
	ins_vstring("C_LVALUE(fp + %i);\n",
		    expr->v.number + current_num_values);
	break;
    case F_LOCAL:
    case NODE_VALUE:
	ins_vstring("C_LOCAL(%i);\n", expr->v.number);
	break;
    case F_LOCAL_LVALUE:
    case NODE_LVALUE:
	ins_vstring("C_LVALUE(fp + %i);\n", expr->v.number);
	break;
    case NODE_IF:
	if (expr->v.expr->kind == F_NOT) {
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
    case NODE_FOR:
	{
	    int forever = node_always_true(expr->l.expr->v.expr);
	    int pos;

	    c_save_loop_info();
	    c_generate_node(expr->l.expr->l.expr);
	    if (!forever) 
		c_generate_forward_branch(F_BRANCH);
	    pos = add_label();
	    c_generate_node(expr->r.expr);
	    c_update_continues();
	    c_generate_node(expr->l.expr->r.expr);
	    if (!forever)
		c_update_forward_branch();
	    if (expr->l.expr->v.expr->kind == F_LOOP_COND){
	        c_generate_node(expr->l.expr->v.expr);
		ins_vstring("goto label%03i;\n", pos);
		notreached = 1;
	    } else
	        c_branch_backwards(generate_conditional_branch(expr->l.expr->v.expr), pos);
	    c_update_breaks();
	    c_restore_loop_info();
	}
	break;
    case NODE_WHILE:
	{
	    int forever = node_always_true(expr->l.expr);
	    int pos;
	    c_save_loop_info();
	    if (!forever)
		c_generate_forward_branch(F_BRANCH);
	    pos = add_label();
	    c_generate_node(expr->r.expr);
	    if (!forever)
		c_update_forward_branch();
	    c_update_continues();
	    if (expr->l.expr->kind == F_LOOP_COND){
	        c_generate_node(expr->l.expr);
		ins_vstring("goto label%03i;\n", pos);
		notreached = 1;
	    } else 
	        c_branch_backwards(generate_conditional_branch(expr->l.expr), pos);
	    c_update_breaks();
	    c_restore_loop_info();
	}
	break;
    case NODE_DO_WHILE:
        {
	    int pos;
            c_save_loop_info();
	    pos = add_label();
            c_generate_node(expr->l.expr);
            c_update_continues();
            c_branch_backwards(generate_conditional_branch(expr->r.expr), pos);
            c_update_breaks();
            c_restore_loop_info();
	}
        break;
    case NODE_CASE_NUMBER:
	case_table_size++;
	notreached = 0;
	if (switch_type == NODE_SWITCH_RANGES) {
	    ins_vstring("case %i:\n", case_number);
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
	} else ins_vstring("case %i:\n", expr->r.number);
	break;
    case NODE_CASE_STRING:
	notreached = 0;
	case_table_size++;
	expr->v.number = case_number;
	ins_vstring("case %i:\n", case_number++);
	break;
    case NODE_DEFAULT:
	notreached = 0;
	ins_string("default:\n");
	break;
    case NODE_SWITCH_STRINGS:
    case NODE_SWITCH_NUMBERS:
    case NODE_SWITCH_RANGES:
    case NODE_SWITCH_DIRECT:
	{
	    int addr;
	    parse_node_t *pn;
	    int save_switch_type = switch_type;
	    int save_case_number = case_number;
	    int save_case_table_size = case_table_size;
	    int *p, *stable;

	    switch_type = expr->kind;
	    case_number = 0;
	    case_table_size = 0;

	    c_generate_node(expr->l.expr);
	    
	    switch (switch_type) {
	    case NODE_SWITCH_STRINGS:
		if (pragmas & PRAGMA_EFUN)
		    ins_string("CHECK_SWITCHES;\n");
		ins_vstring("i = c_string_switch_lookup(sp, string_switch_table_%s_%02i);\nfree_string_svalue(sp--);\n", compilation_ident, string_switches++);
		break;
	    case NODE_SWITCH_DIRECT:
	    case NODE_SWITCH_NUMBERS:
		ins_string("i = (sp--)->u.number;\n");
		break;
	    case NODE_SWITCH_RANGES:
		ins_vstring("i = c_range_switch_lookup((sp--)->u.number, %i);\n", range_switches++);
		break;
	    }
	    ins_string("switch (i) {\n");
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

            switch_type = save_switch_type;
	    case_number = save_case_number;
	    case_table_size = save_case_table_size;
	    break;
	}
    case NODE_CONDITIONAL:
	{
	    int addr1, addr2;
	    
	    c_generate_node(expr->l.expr);
	    ins_string("C_CHECK_TRUE();\nif (!i) ");
	    addr1 = ins_jump();
	    notreached = 0;

	    c_generate_node(expr->r.expr->l.expr);

	    addr2 = ins_jump();
	    upd_jump(addr1, add_label());
	    
	    c_generate_node(expr->r.expr->r.expr);
	    upd_jump(addr2, add_label());
	}
	break;
    case F_CATCH:
	{
	    int addr;

	    ins_string("if (c_catch()) {\n");
	    c_generate_node(expr->r.expr);
	    ins_string("c_end_catch();\n}\n");
	    break;
	}
    case F_SSCANF:
	c_generate_node(expr->l.expr->l.expr);
	c_generate_node(expr->l.expr->r.expr);
	ins_vstring("c_sscanf(%i);\n", expr->r.expr->v.number);
	generate_lvalue_list(expr->r.expr);
	break;
    case F_PARSE_COMMAND:
	c_generate_node(expr->l.expr->l.expr);
	c_generate_node(expr->l.expr->r.expr->l.expr);
	c_generate_node(expr->l.expr->r.expr->r.expr);
	ins_vstring("c_parse_command(%i);\n", expr->r.expr->v.number);
	generate_lvalue_list(expr->r.expr);
	break;
    case F_TIME_EXPRESSION:
	ins_string("f_time_expression();\n");
	c_generate_node(expr->r.expr);
	ins_string("f_end_time_expression();\n");
	break;
    case F_TO_FLOAT:
	generate_expr_list(expr->r.expr);
	ins_string("CHECK_TYPES(sp, T_STRING | T_FLOAT | T_NUMBER, 1, F_TO_FLOAT);\nf_to_float();\n");
	break;
    case F_TO_INT:
	generate_expr_list(expr->r.expr);
	ins_string("CHECK_TYPES(sp, T_STRING | T_FLOAT | T_NUMBER | T_BUFFER, 1, F_TO_INT);\nf_to_int();\n");
	break;
    case F_GLOBAL_LVALUE:
	if (pragmas & PRAGMA_EFUN)
	    yyerror("Ilegal to use a global in an efun.\n");
	ins_vstring("C_LVALUE(&current_object->variables[variable_index_offset + %i]);\n", expr->v.number);
	break;
    case F_GLOBAL:
	if (pragmas & PRAGMA_EFUN)
	    yyerror("Ilegal to use a global in an efun.\n");
	ins_vstring("C_GLOBAL(%i);\n", expr->v.number);
	break;
    case F_LOOP_INCR:
	ins_vstring("C_LOOP_INCR(%i);\n", expr->v.number);
	break;
    case F_WHILE_DEC:
	ins_vstring("C_WHILE_DEC(%i); if (i)\n", expr->v.number);
	break;
    case F_LOOP_COND:
	{
	    int i;
	    
	    if (expr->r.expr->kind == F_LOCAL)
		ins_vstring("C_LOOP_COND_LV(%i, %i); if (i)\n", 
			    expr->l.expr->v.number, expr->r.expr->v.number);
		else {
		    switch (expr->r.expr->kind) {
		    case F_CONST0: i = 0; break;
		    case F_CONST1: i = 1; break;
		    case F_NBYTE: i = - expr->r.expr->v.number; break;
		    case F_BYTE:
		    case F_NUMBER:
			i = expr->r.expr->v.number; break;
		    default:
			fatal("Unknown node %i in F_LOOP_COND\n",
			      expr->r.expr->kind);
		    }
		    ins_vstring("C_LOOP_COND_NUM(%i, %i); if (i)\n", 
				expr->l.expr->v.number, i);
		}
	    break;
	}
    case F_SIMUL_EFUN:
	if (pragmas & PRAGMA_EFUN)
	    yyerror("Ilegal to use local function calls or simul_efuns in an efun.\n");
	generate_expr_list(expr->r.expr);
	ins_vstring("call_simul_efun(%i, %i);\n",
		    expr->v.number, expr->r.expr ? expr->r.expr->kind : 0);
	break;
    case F_CALL_FUNCTION_BY_ADDRESS:
	if (pragmas & PRAGMA_EFUN)
	    yyerror("Ilegal to use local function calls or simul_efuns in an efun.\n");
	generate_expr_list(expr->r.expr);
	ins_vstring("c_call(%i, %i);\n", 
		    expr->v.number, expr->r.expr ? expr->r.expr->kind : 0);
	break;
    case F_CALL_INHERITED:
	if (pragmas & PRAGMA_EFUN)
	    yyerror("Ilegal to call an inherited function in an efun.\n");
	generate_expr_list(expr->r.expr);
	ins_vstring("c_call_inherited(%i, %i, %i);\n", 
		    expr->v.number & 0xff, expr->v.number >> 8,
		    expr->r.expr ? expr->r.expr->kind : 0);
	break;
    case F_EVALUATE:
	generate_expr_list(expr->r.expr);
	ins_vstring("c_evaluate(%i);\n", expr->v.number);
	break;
    case F_FUNCTION_CONSTRUCTOR:
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
/*		int addr, save_current_num_values = current_num_values;
		ins_byte(expr->v.number >> 8);
		addr = CURRENT_PROGRAM_SIZE;
		ins_short(0);
		current_num_values = expr->r.expr ? expr->r.expr->kind : 0;
		c_generate_node(expr->l.expr);
		current_num_values = save_current_num_values;
		ins_f_byte(F_RETURN);
		upd_short(addr, CURRENT_PROGRAM_SIZE - addr - 2);
		break; */
	    }
	}
	break;
    case NODE_ANON_FUNC:
	{
	    int addr;
	    
/*	    ins_f_byte(F_FUNCTION_CONSTRUCTOR);
	    ins_byte(FP_ANONYMOUS);
	    ins_byte(expr->v.number);
	    ins_byte(expr->l.number);
	    addr = CURRENT_PROGRAM_SIZE;
	    ins_short(0);
	    c_generate_node(expr->r.expr);
	    upd_short(addr, CURRENT_PROGRAM_SIZE - addr - 2);
	    break; */
	}
    default:
	{
	    parse_node_t *node = expr->r.expr;
	    int num_arg = expr->v.number;
	    int f = expr->kind;

	    DEBUG_CHECK1(f < BASE,
			 "Unknown eoperator %s in c_generate_node.\n",
			 get_f_name(expr->kind));
	    if (num_arg == -1)
		num_arg = instrs[f].min_arg;

	    generate_expr_list(node);
	    if (node && node->v.expr) {
		ins_vstring("CHECK_TYPES(sp - %i, %i, 1, %i);\n",
			    num_arg - 1, instrs[f].type[0], f);
		if ((node = node->r.expr) && node->v.expr) {
		ins_vstring("CHECK_TYPES(sp - %i, %i, 2, %i);\n",
			    num_arg - 2, instrs[f].type[1], f);
		}
	    }
	    if (expr->v.number != -1)
		ins_vstring("st_num_arg = %i;\n", expr->v.number);
	    ins_vstring("f_%s();\n", instrs[f].name);
	    if (expr->type == TYPE_NOVALUE) {
		/* the value of a void efun was used.  Put in a zero. */
		ins_string("push_number(0);\n");
	    }
	}
   }
}

void
c_generate_inherited_init_call P2(int, index, short, f) {
    ins_vstring("c_call_inherited(%i, %i, 0);\npop_stack();\n", index, (int)f);
}

void c_start_function P1(function_t *, f) {
    notreached = 0;
    if (pragmas & PRAGMA_EFUN) {
	current_frame_size = f->num_local + f->num_arg;
	ins_vstring("void f_%s PROT((void)) {\n", f->name);
	if (f->type & TYPE_MOD_VARARGS) {
	    ins_vstring("svalue_t *fp = sp - st_num_arg + 1;\npush_nulls(%i - st_num_arg);\n", f->num_arg + f->num_local);
	} else if (f->num_local) {
	    ins_vstring("svalue_t *fp = sp - %i + 1;\npush_nulls(%i);\n", f->num_arg, f->num_local);
	} else if (f->num_arg) {
	    ins_vstring("svalue_t *fp = sp - %i + 1;\n", f->num_arg);
	}
    } else
	ins_vstring("static void LPC_%s__%s() {\n", compilation_ident, f->name);
}

void c_end_function() {
    notreached = 0;
    ins_string("}\n\n");
}

void c_generate___INIT() {
    fprintf(f_out, "static void LPCINIT_%s() {\n", compilation_ident);
    fwrite(mem_block[A_INITIALIZER].block, 
	   mem_block[A_INITIALIZER].current_size, 1, f_out);
    fprintf(f_out, "}\n");
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
    } 
}

static void
c_update_breaks() {
  /* traverse the list of nodes filling in the break address
     required by each "break" statement.
     */
  while (!break_ptr->type){
      upd_jump(break_ptr->v.number, label);
      break_ptr = break_ptr->r.expr;
  }
  add_label();
}

static void
c_update_continues() {
  /* traverse the linked list filling in the continue address
     required by each "continue" statement.
     */
  while (!cont_ptr->type){
      upd_jump(cont_ptr->v.number, label);
      cont_ptr = cont_ptr->r.expr;
  }
  add_label();
}

static void
c_save_loop_info() {
    /* Deactivate the current break and cont pointers */
    break_ptr->type = 1;
    cont_ptr->type = 1;
}

static void
c_restore_loop_info() {
    /* Reactivate the current break and cont pointers */
    if (cont_ptr != &cont_dummy) cont_ptr->type = 0;
    if (break_ptr != &break_dummy) break_ptr->type = 0;
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
    break_ptr = &break_dummy;
    cont_ptr = &cont_dummy;
    forward_branch_ptr = &forward_branch_stack[0];

    current_block = A_PROGRAM;
    prog_code = mem_block[A_PROGRAM].block;
    prog_code_max = mem_block[A_PROGRAM].block + mem_block[A_PROGRAM].max_size;

    string_switches = range_switches = 0;
    switch_tables = 0;
    label = 0;
    notreached = 0;
}

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

void
c_generate_final_program P1(int, x) {
    switch_table_t *st, *next;
    function_t *funp;
    int i;
    int index = 0;

    if (!x) {
	if (pragmas & PRAGMA_EFUN)
	    fprintf(f_out, "#include \"../lpc_to_c.h\"\n\n");
	else
	    fprintf(f_out, "#include \"lpc_to_c.h\"\n\n");
	if (string_switches) {
	    if (pragmas & PRAGMA_EFUN) {
		fprintf(f_out, "int switches_need_fixing = 1;\n\n#define NUM_STRING_SWITCHES %i\n\n", string_switches);
	    }
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
	    fprintf(f_out, "static string_switch_entry_t *string_switch_tables_%s[] = {\n", compilation_ident);
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
		    fprintf(f_out, "{ 0, 0 }\n};\n\n");
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

	fwrite(mem_block[A_PROGRAM].block,
	       mem_block[A_PROGRAM].current_size, 1, f_out);

	current_block = A_PROGRAM;
	prog_code = mem_block[A_PROGRAM].block;

	fprintf(f_out, "\n\nvoid (*LPCFUNCS_%s[])() = {\n", compilation_ident);
	for (i = 0; i < mem_block[A_FUNCTIONS].current_size; i += sizeof *funp) {
	    funp = (function_t *)(mem_block[A_FUNCTIONS].block + i);
	    if (!(funp->flags & NAME_NO_CODE))
		fprintf(f_out, "LPC_%s__%s,\n", compilation_ident, funp->name);
	}
	fprintf(f_out, "0\n};\n");
    }
}
#endif
