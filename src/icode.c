/* 
 * code generator for runtime LPC code
 */
#include "std.h"
#include "lpc_incl.h"
#include "icode.h"
#include "compiler.h"
#include "generate.h"

static void ins_real PROT((double));
static void ins_short PROT((short));
static void upd_short PROT((int, short));
static void ins_byte PROT((unsigned char));
static void upd_byte PROT((int, unsigned char));
static void write_number PROT((int));
static short read_short PROT((int));
static void ins_int PROT((int));
#if SIZEOF_PTR == 8
static void ins_long PROT((long));
#endif
void i_generate_node PROT((parse_node_t *));
static void i_generate_if_branch PROT((parse_node_t *, int));
static void i_generate_loop PROT((int, parse_node_t *, parse_node_t *, 
				  parse_node_t *));
static void i_update_branch_list PROT((parse_node_t *));
static int try_to_push PROT((int, int));

/*
   this variable is used to properly adjust the 'break_sp' stack in
   the event a 'continue' statement is issued from inside a 'switch'.
*/
static int foreach_depth = 0;

static int current_forward_branch;
static int current_num_values;

static int last_size_generated;
static int line_being_generated;

static int push_state;
static int push_start;

static parse_node_t *branch_list[3];

static void ins_real P1(double, l)
{
    float f = (float)l;

    if (prog_code + 4 > prog_code_max) {
	mem_block_t *mbp = &mem_block[current_block];

	UPDATE_PROGRAM_SIZE;
	realloc_mem_block(mbp, mbp->current_size * 2);
	
	prog_code = mbp->block + mbp->current_size;
	prog_code_max = mbp->block + mbp->max_size;
    }
    STORE_FLOAT(prog_code, f);
}

/*
 * Store a 2 byte number. It is stored in such a way as to be sure
 * that correct byte order is used, regardless of machine architecture.
 * Also beware that some machines can't write a word to odd addresses.
 */
static void ins_short P1(short, l)
{
    if (prog_code + 2 > prog_code_max) {
	mem_block_t *mbp = &mem_block[current_block];
	UPDATE_PROGRAM_SIZE;
	realloc_mem_block(mbp, mbp->current_size * 2);
	
	prog_code = mbp->block + mbp->current_size;
	prog_code_max = mbp->block + mbp->max_size;
    }
    STORE_SHORT(prog_code, l);
}

static short read_short P1(int, offset)
{
    short l;

    COPY_SHORT(&l, mem_block[current_block].block + offset);
    return l;
}

/*
 * Store a 4 byte number. It is stored in such a way as to be sure
 * that correct byte order is used, regardless of machine architecture.
 */
static void ins_int P1(int, l)
{

    if (prog_code + 4 > prog_code_max) {
	mem_block_t *mbp = &mem_block[current_block];
	UPDATE_PROGRAM_SIZE;
	realloc_mem_block(mbp, mbp->current_size * 2);
	
	prog_code = mbp->block + mbp->current_size;
	prog_code_max = mbp->block + mbp->max_size;
    }
    STORE_INT(prog_code, l);
}

/*
 * Store a 8 byte number. It is stored in such a way as to be sure
 * that correct byte order is used, regardless of machine architecture.
 */
#if SIZEOF_PTR == 8
static void ins_long P1(long, l)
{
    if (prog_code + 8 > prog_code_max) {
	mem_block_t *mbp = &mem_block[current_block];
	UPDATE_PROGRAM_SIZE;
	realloc_mem_block(mbp, mbp->current_size * 2);
	
	prog_code = mbp->block + mbp->current_size;
	prog_code_max = mbp->block + mbp->max_size;
    }
    STORE_PTR(prog_code, l);
}
#endif

static void upd_short P2(int, offset, short, l)
{
    IF_DEBUG(UPDATE_PROGRAM_SIZE);
    DEBUG_CHECK2(offset > CURRENT_PROGRAM_SIZE,
		 "patch offset %x larger than current program size %x.\n",
		 offset, CURRENT_PROGRAM_SIZE);
    COPY_SHORT(mem_block[current_block].block + offset, &l);
}

static void ins_byte P1(unsigned char, b)
{
    if (prog_code == prog_code_max) {
	mem_block_t *mbp = &mem_block[current_block];
	UPDATE_PROGRAM_SIZE;
	realloc_mem_block(mbp, mbp->current_size * 2);
	
	prog_code = mbp->block + mbp->current_size;
	prog_code_max = mbp->block + mbp->max_size;
    }
    *prog_code++ = b;
}

static void upd_byte P2(int, offset, unsigned char, b)
{
    IF_DEBUG(UPDATE_PROGRAM_SIZE);
    DEBUG_CHECK2(offset > CURRENT_PROGRAM_SIZE,
		"patch offset %x larger than current program size %x.\n",
		offset, CURRENT_PROGRAM_SIZE);
    mem_block[current_block].block[offset] = b;
}

static void end_pushes PROT((void)) {
    if (push_state) {
	if (push_state > 1)
	    upd_byte(push_start, push_state);
	push_state = 0;
    }
}

static void initialize_push PROT((void)) {
    int what = mem_block[current_block].block[push_start];
    int arg = mem_block[current_block].block[push_start + 1];

    prog_code = mem_block[current_block].block + push_start;
    ins_byte(F_PUSH);
    push_start++; /* now points to the zero here */
    ins_byte(0);
    
    switch (what) {
    case F_CONST0:
	ins_byte(PUSH_NUMBER | 0);
	break;
    case F_CONST1:
	ins_byte(PUSH_NUMBER | 1);
	break;
    case F_BYTE:
	ins_byte(PUSH_NUMBER | arg);
	break;
    case F_SHORT_STRING:
	ins_byte(PUSH_STRING | arg);
	break;
    case F_LOCAL:
	ins_byte(PUSH_LOCAL | arg);
	break;
    case F_GLOBAL:
	ins_byte(PUSH_GLOBAL | arg);
	break;
    }
}

/*
 * Generate the code to push a number on the stack.
 * This varies since there are several opcodes (for
 * optimizing speed and/or size).
 */
static void write_small_number P1(int, val) {
    if (try_to_push(PUSH_NUMBER, val)) return;
    ins_byte(F_BYTE);
    ins_byte(val);
}

static void write_number P1(int, val)
{
    if ((val & ~0xff) == 0)
	write_small_number(val);
    else {
	end_pushes();
	if (val < 0 && val > -256) {
	    ins_byte(F_NBYTE);
	    ins_byte(-val);
	} else {
	    ins_byte(F_NUMBER);
	    ins_int(val);
	}
    }
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
	i_generate_node(pn->v.expr);
	n++;
    } while ((pn = pn->r.expr));
    
    if (flag) {
	pn = expr;
	do {
	    n--;
	    if (pn->type & 1) {
		end_pushes();
		ins_byte(F_EXPAND_VARARGS);
		ins_byte(n);
	    }
	} while ((pn = pn->r.expr));
    }
}

static void
generate_lvalue_list P1(parse_node_t *, expr) {
    while ((expr = expr->r.expr)) {
      i_generate_node(expr->l.expr);
      end_pushes();
      ins_byte(F_VOID_ASSIGN);
    }
}

INLINE void
switch_to_line P1(int, line) {
    int sz = CURRENT_PROGRAM_SIZE - last_size_generated;
    short s;
    unsigned char *p;

    /* should be fixed later */
    if (current_block != A_PROGRAM)
	return;

    if (sz) {
	s = line_being_generated;

	last_size_generated += sz;
	while (sz > 255) {
	    p = (unsigned char *)allocate_in_mem_block(A_LINENUMBERS, 3);
	    *p++ = 255;
	    STORE_SHORT(p, s);
	    sz -= 255;
	}
	p = (unsigned char *)allocate_in_mem_block(A_LINENUMBERS, 3);
	*p++ = sz;
	STORE_SHORT(p, s);
    }
    line_being_generated = line;
}

static int
try_to_push P2(int, kind, int, value) {
    if (push_state) {
	if (value <= PUSH_MASK) {
	    if (push_state == 1)
		initialize_push();
	    push_state++;
	    ins_byte(kind | value);
	    if (push_state == 255)
		end_pushes();
	    return 1;
	} else end_pushes();
    } else if (value <= PUSH_MASK) {
	push_start = CURRENT_PROGRAM_SIZE;
	push_state = 1;
	switch (kind) {
	case PUSH_STRING: ins_byte(F_SHORT_STRING); break;
	case PUSH_LOCAL: ins_byte(F_LOCAL); break;
	case PUSH_GLOBAL: ins_byte(F_GLOBAL); break;
	case PUSH_NUMBER: 
	    if (value == 0) {
		ins_byte(F_CONST0);
		return 1;
	    } else if (value == 1) {
		ins_byte(F_CONST1);
		return 1;
	    }
	    ins_byte(F_BYTE);
	}
	ins_byte(value);
	return 1;
    }
    return 0;
}

void
i_generate_node P1(parse_node_t *, expr) {
    if (!expr) return;
    
    if (expr->line && expr->line != line_being_generated)
	switch_to_line(expr->line);
    switch (expr->kind) {
    case NODE_TERNARY_OP:
	i_generate_node(expr->l.expr);
	expr = expr->r.expr;
    case NODE_BINARY_OP:
	i_generate_node(expr->l.expr);
	/* fall through */
    case NODE_UNARY_OP:
	i_generate_node(expr->r.expr);
	/* fall through */
    case NODE_OPCODE:
	end_pushes();
	ins_byte(expr->v.number);
	break;
    case NODE_TERNARY_OP_1:
	i_generate_node(expr->l.expr);
	expr = expr->r.expr;
	/* fall through */
    case NODE_BINARY_OP_1:
	i_generate_node(expr->l.expr);
	i_generate_node(expr->r.expr);
	end_pushes();
	ins_byte(expr->v.number);
	ins_byte(expr->type);
	break;
    case NODE_UNARY_OP_1:
	i_generate_node(expr->r.expr);
	/* fall through */
    case NODE_OPCODE_1:
	if (expr->v.number == F_LOCAL) {
	    if (try_to_push(PUSH_LOCAL, expr->l.number)) break;
	} else if (expr->v.number == F_GLOBAL) {
	    if (try_to_push(PUSH_GLOBAL, expr->l.number)) break;
	}
	end_pushes();
	ins_byte(expr->v.number);
	ins_byte(expr->l.number);
	break;
    case NODE_OPCODE_2:
	end_pushes();
	ins_byte(expr->v.number);
	ins_byte(expr->l.number);
	if (expr->v.number == F_LOOP_COND_NUMBER)
	    ins_int(expr->r.number);
	else ins_byte(expr->r.number);
	break;
    case NODE_RETURN:
	{
	    int n;
	    n = foreach_depth;
	    end_pushes();
	    while (n--)
		ins_byte(F_EXIT_FOREACH);

	    if (expr->r.expr) {
		i_generate_node(expr->r.expr);
		end_pushes();
		ins_byte(F_RETURN);
	    } else ins_byte(F_RETURN_ZERO);
	    break;
	}
    case NODE_STRING:
	if (try_to_push(PUSH_STRING, expr->v.number)) break;
	if (expr->v.number <= 0xff) {
	    ins_byte(F_SHORT_STRING);
	    ins_byte(expr->v.number);
	} else {
	    ins_byte(F_STRING);
	    ins_short(expr->v.number);
	}
	break;
    case NODE_REAL:
	end_pushes();
	ins_byte(F_REAL);
	ins_real(expr->v.real);
	break;
    case NODE_NUMBER:
	write_number(expr->v.number);
	break;
    case NODE_LAND_LOR:
	i_generate_node(expr->l.expr);
	i_generate_forward_branch(expr->v.number);
	i_generate_node(expr->r.expr);
	if (expr->l.expr->kind == NODE_BRANCH_LINK) {
	    i_update_forward_branch_links(expr->v.number,expr->l.expr);
	}
	else i_update_forward_branch();
	break;
    case NODE_BRANCH_LINK:
	i_generate_node(expr->l.expr);
	end_pushes();
	ins_byte(0);
	expr->v.number = CURRENT_PROGRAM_SIZE;
	ins_short(0);
	i_generate_node(expr->r.expr);
        break;
    case NODE_CALL_2:
	generate_expr_list(expr->r.expr);
	end_pushes();
	ins_byte(expr->v.number);
	ins_byte(expr->l.number >> 16);
	ins_short(expr->l.number & 0xffff);
	ins_byte((expr->r.expr ? expr->r.expr->kind : 0));
	break;
    case NODE_CALL_1:
	generate_expr_list(expr->r.expr);
	end_pushes();
	ins_byte(expr->v.number);
	ins_short(expr->l.number);
	ins_byte((expr->r.expr ? expr->r.expr->kind : 0));
	break;
    case NODE_CALL:
	generate_expr_list(expr->r.expr);
	end_pushes();
	ins_byte(expr->v.number);
	ins_short(expr->l.number);
	break;
    case NODE_TWO_VALUES:
	i_generate_node(expr->l.expr);
	i_generate_node(expr->r.expr);
	break;
    case NODE_CONTROL_JUMP:
	{
	    int kind = expr->v.number;
	    end_pushes();
	    ins_byte(F_BRANCH);
	    expr->v.expr = branch_list[kind];
	    expr->l.number = CURRENT_PROGRAM_SIZE;
	    ins_short(0);
	    branch_list[kind] = expr;
	    break;
	}
    case NODE_PARAMETER:
	{
	    int which = expr->v.number + current_num_values;
	    if (try_to_push(PUSH_LOCAL, which)) break;
	    ins_byte(F_LOCAL);
	    ins_byte(which);
	    break;
	}
    case NODE_PARAMETER_LVALUE:
	end_pushes();
	ins_byte(F_LOCAL_LVALUE);
	ins_byte(expr->v.number + current_num_values);
	break;
    case NODE_IF:
	i_generate_if_branch(expr->v.expr, 0);
	i_generate_node(expr->l.expr);
	if (expr->r.expr) {
	    i_generate_else();
	    i_generate_node(expr->r.expr);
	}
	i_update_forward_branch();
	break;
    case NODE_LOOP:
	i_generate_loop(expr->type, expr->v.expr, expr->l.expr, expr->r.expr);
	break;
    case NODE_FOREACH:
	{
	    int tmp = 0;

	    i_generate_node(expr->v.expr);
	    end_pushes();
	    ins_byte(F_FOREACH);
	    if (expr->l.expr->v.number == F_GLOBAL_LVALUE) 
		tmp |= FOREACH_RIGHT_GLOBAL;
	    else if (expr->l.expr->v.number == F_REF_LVALUE)
		tmp |= FOREACH_REF;
	    if (expr->r.expr) {
		if (tmp & FOREACH_RIGHT_GLOBAL)
		    tmp = (tmp & ~FOREACH_RIGHT_GLOBAL) | FOREACH_LEFT_GLOBAL;

		tmp |= FOREACH_MAPPING;
		if (expr->r.expr->v.number == F_GLOBAL_LVALUE) 
		    tmp |= FOREACH_RIGHT_GLOBAL;
		else if (expr->r.expr->v.number == F_REF_LVALUE)
		    tmp |= FOREACH_REF;
	    }
	    ins_byte(tmp);
	    ins_byte(expr->l.expr->l.number);
	    if (expr->r.expr)
		ins_byte(expr->r.expr->l.number);
	}
	break;
    case NODE_CASE_NUMBER:
    case NODE_CASE_STRING:
	if (expr->v.expr) {
	    parse_node_t *other = expr->v.expr;
	    expr->v.number = 1;
	    other->l.expr = expr->l.expr;
	    other->v.number = CURRENT_PROGRAM_SIZE;
	    expr->l.expr = other;
	} else {
	    expr->v.number = CURRENT_PROGRAM_SIZE;
	}
	end_pushes();
	break;
    case NODE_DEFAULT:
	expr->v.number = CURRENT_PROGRAM_SIZE;
	end_pushes();
	break;
    case NODE_SWITCH_STRINGS:
    case NODE_SWITCH_NUMBERS:
    case NODE_SWITCH_DIRECT:
    case NODE_SWITCH_RANGES:
	{
	    int addr, last_break;
	    parse_node_t *sub = expr->l.expr;
	    parse_node_t *save_switch_breaks = branch_list[CJ_BREAK_SWITCH];
	    
	    i_generate_node(sub);
	    branch_list[CJ_BREAK_SWITCH] = 0;
	    end_pushes();
	    ins_byte(F_SWITCH);
	    ins_byte(0xff); /* kind of table */
	    addr = CURRENT_PROGRAM_SIZE;
	    ins_short(0); /* address of table */
	    ins_short(0); /* end of table */
	    ins_short(0); /* default address */
	    i_generate_node(expr->r.expr);
	    if (expr->v.expr && expr->v.expr->kind == NODE_DEFAULT) {
		upd_short(addr + 4, expr->v.expr->v.number);
		expr->v.expr = expr->v.expr->l.expr;
	    } else {
		upd_short(addr + 4, CURRENT_PROGRAM_SIZE);
	    }
	    /* just in case the last case doesn't have a break */
	    end_pushes();
	    ins_byte(F_BRANCH);
	    last_break = CURRENT_PROGRAM_SIZE;
	    ins_short(0);
	    /* build table */
	    upd_short(addr, CURRENT_PROGRAM_SIZE);
#ifdef BINARIES
	    if (expr->kind == NODE_SWITCH_STRINGS) {
		short sw;
		sw = addr - 2;
		add_to_mem_block(A_PATCH, (char *)&sw, sizeof sw);
	    }
#endif
	    if (expr->kind == NODE_SWITCH_DIRECT) {
		parse_node_t *pn = expr->v.expr;
		while (pn) {
		    ins_short((short)pn->v.number);
		    pn = pn->l.expr;
		}
		ins_int(expr->v.expr->r.number);
		mem_block[current_block].block[addr-1] = (char)0xfe;
	    } else {
		int table_size = 0;
		int power_of_two = 1;
		int i = 0;
		parse_node_t *pn = expr->v.expr;
		
		while (pn) {
		    if (expr->kind == NODE_SWITCH_STRINGS) {
			if (pn->r.number) {
			    INS_POINTER((POINTER_INT)
					PROG_STRING(pn->r.number));
			} else 
			    INS_POINTER((POINTER_INT)0);
		    } else
			INS_POINTER((POINTER_INT)pn->r.expr);
		    ins_short((short)pn->v.number);
		    pn = pn->l.expr;
		    table_size += 1;
		}
		while ((power_of_two<<1) <= table_size) {
		    power_of_two <<= 1;
		    i++;
		}
		if (expr->kind != NODE_SWITCH_STRINGS)
		    mem_block[current_block].block[addr-1] = (char)(0xf0+i);
		else
		    mem_block[current_block].block[addr-1] = (char)(i*0x10+0x0f);
	    }
	    i_update_branch_list(branch_list[CJ_BREAK_SWITCH]);
	    branch_list[CJ_BREAK_SWITCH] = save_switch_breaks;
	    upd_short(last_break, CURRENT_PROGRAM_SIZE - last_break);
	    upd_short(addr+2, CURRENT_PROGRAM_SIZE);
	    break;
	}
    case NODE_CATCH:
	{
	    int addr;

	    end_pushes();
	    ins_byte(F_CATCH);
	    addr = CURRENT_PROGRAM_SIZE;
	    ins_short(0);
	    i_generate_node(expr->r.expr);
	    ins_byte(F_END_CATCH);
	    upd_short(addr, CURRENT_PROGRAM_SIZE - addr);
	    break;
	}
    case NODE_TIME_EXPRESSION:
	{
	    end_pushes();
	    ins_byte(F_TIME_EXPRESSION);
	    i_generate_node(expr->r.expr);
	    ins_byte(F_END_TIME_EXPRESSION);
	    break;
	}
    case NODE_LVALUE_EFUN:
	i_generate_node(expr->l.expr);
	generate_lvalue_list(expr->r.expr);
	break;
    case NODE_FUNCTION_CONSTRUCTOR:
	if (expr->r.expr) {
	    generate_expr_list(expr->r.expr);
	    end_pushes();
	    ins_byte(F_AGGREGATE);
	    ins_short(expr->r.expr->kind);
	} else {
	    end_pushes();
	    ins_byte(F_CONST0);
	}
	end_pushes();
	ins_byte(F_FUNCTION_CONSTRUCTOR);
	ins_byte(expr->v.number & 0xff);

	switch (expr->v.number & 0xff) {
	case FP_SIMUL:
	case FP_LOCAL:
	    ins_short(expr->v.number >> 8);
	    break;
	case FP_EFUN:
	    ins_short(predefs[expr->v.number >> 8].token);
	    break;
	case FP_FUNCTIONAL:
	case FP_FUNCTIONAL | FP_NOT_BINDABLE:
	    {
		int addr, save_current_num_values = current_num_values;
		ins_byte(expr->v.number >> 8);
		addr = CURRENT_PROGRAM_SIZE;
		ins_short(0);
		current_num_values = expr->r.expr ? expr->r.expr->kind : 0;
		i_generate_node(expr->l.expr);
		current_num_values = save_current_num_values;
		end_pushes();
		ins_byte(F_RETURN);
		upd_short(addr, CURRENT_PROGRAM_SIZE - addr - 2);
		break;
	    }
	}
	break;
    case NODE_ANON_FUNC:
	{
	    int addr;
	    int save_fd = foreach_depth;

	    foreach_depth = 0;
	    end_pushes();
	    ins_byte(F_FUNCTION_CONSTRUCTOR);
	    if (expr->v.number & 0x10000)
		ins_byte(FP_ANONYMOUS | FP_NOT_BINDABLE);
	    else
		ins_byte(FP_ANONYMOUS);
	    ins_byte(expr->v.number & 0xff);
	    ins_byte(expr->l.number);
	    addr = CURRENT_PROGRAM_SIZE;
	    ins_short(0);
	    i_generate_node(expr->r.expr);
	    upd_short(addr, CURRENT_PROGRAM_SIZE - addr - 2);
	    foreach_depth = save_fd;
	    break;
	}
    case NODE_EFUN:
	{
	    int novalue_used = expr->v.number & NOVALUE_USED_FLAG;
	    int f = expr->v.number & ~NOVALUE_USED_FLAG;
	    
	    generate_expr_list(expr->r.expr);
	    end_pushes();
	    if (f < ONEARG_MAX) {
		ins_byte(f);
	    } else {
		/* max_arg == -1 must use F_EFUNV so that varargs expansion works*/
		if (expr->l.number < 4 && instrs[f].max_arg != -1)
		    ins_byte(F_EFUN0 + expr->l.number);
		else {
		    ins_byte(F_EFUNV);
		    ins_byte(expr->l.number);
		}
		ins_byte(f - ONEARG_MAX);
	    }
	    if (novalue_used) {
		/* the value of a void efun was used.  Put in a zero. */
		ins_byte(F_CONST0);
	    }
	    break;
	default:
	    fatal("Unknown node %i in i_generate_node.\n", expr->kind);
	}
    }
}

static void i_generate_loop P4(int, test_first, parse_node_t *, block,
			       parse_node_t *, inc, parse_node_t *, test) {
    parse_node_t *save_breaks = branch_list[CJ_BREAK];
    parse_node_t *save_continues = branch_list[CJ_CONTINUE];
    int forever = node_always_true(test);
    int pos;
    
    if (test_first == 2) foreach_depth++;
    branch_list[CJ_BREAK] = branch_list[CJ_CONTINUE] = 0;
    end_pushes();
    if (!forever && test_first)
	i_generate_forward_branch(F_BRANCH);
    pos = CURRENT_PROGRAM_SIZE;
    i_generate_node(block);
    i_update_branch_list(branch_list[CJ_CONTINUE]);
    if (inc) i_generate_node(inc);
    if (!forever && test_first) i_update_forward_branch();
    if (test->v.number == F_LOOP_COND_LOCAL ||
	test->v.number == F_LOOP_COND_NUMBER ||
	test->v.number == F_NEXT_FOREACH) {
	i_generate_node(test);
	ins_short(CURRENT_PROGRAM_SIZE - pos);
    } else i_branch_backwards(generate_conditional_branch(test), pos);
    i_update_branch_list(branch_list[CJ_BREAK]);
    branch_list[CJ_BREAK] = save_breaks;
    branch_list[CJ_CONTINUE] = save_continues;
    if (test_first == 2) foreach_depth--;
}

static void
i_generate_if_branch P2(parse_node_t *, node, int, invert) {
    int generate_both = 0;
    int branch = (invert ? F_BRANCH_WHEN_NON_ZERO : F_BRANCH_WHEN_ZERO);
    
    switch (node->kind) {
    case NODE_UNARY_OP:
	if (node->v.number == F_NOT) {
	    i_generate_if_branch(node->r.expr, !invert);
	    return;
	}
	break;
    case NODE_BINARY_OP:
	switch (node->v.number) {
	case F_EQ:
	    generate_both = 1;
	    branch = (invert ? F_BRANCH_EQ : F_BRANCH_NE);
	    break;
	case F_GE:
	    if (invert) {
		generate_both = 1;
		branch = F_BRANCH_GE;
	    }
	    break;
	case F_LE:
	    if (invert) {
		generate_both = 1;
		branch = F_BRANCH_LE;
	    }
	    break;
	case F_LT:
	    if (!invert) {
		generate_both = 1;
		branch = F_BRANCH_GE;
	    }
	    break;
	 case F_GT:
	    if (!invert) {
		generate_both = 1;
		branch = F_BRANCH_LE;
	    }
	    break;
	 case F_NE:
	    generate_both = 1;
	    branch = (invert ? F_BRANCH_NE : F_BRANCH_EQ);
	    break;
	}
    }
    if (generate_both) {
	i_generate_node(node->l.expr);
	i_generate_node(node->r.expr);
    } else {
	i_generate_node(node);
    }
    i_generate_forward_branch(branch);
}

void
i_generate_inherited_init_call P2(int, index, short, f) {
    end_pushes();
    ins_byte(F_CALL_INHERITED);
    ins_byte(index);
    ins_short(f);
    ins_byte(0);
    ins_byte(F_POP_VALUE);
}

void i_generate___INIT() {
    add_to_mem_block(A_PROGRAM, (char *)mem_block[A_INITIALIZER].block,
		     mem_block[A_INITIALIZER].current_size);
    prog_code = mem_block[A_PROGRAM].block + mem_block[A_PROGRAM].current_size;
}

void i_generate_forward_branch P1(char, b) {
    end_pushes();
    ins_byte(b);
    ins_short(current_forward_branch);
    current_forward_branch = CURRENT_PROGRAM_SIZE - 2;
}

void
i_update_forward_branch() {
    int i = read_short(current_forward_branch);
    
    end_pushes();
    upd_short(current_forward_branch, CURRENT_PROGRAM_SIZE - current_forward_branch);
    current_forward_branch = i;
}

void i_update_forward_branch_links P2(char, kind, parse_node_t *, link_start){
    int i;

    end_pushes();
    i = read_short(current_forward_branch);
    upd_short(current_forward_branch, CURRENT_PROGRAM_SIZE - current_forward_branch);
    current_forward_branch = i;
    do {
	i = link_start->v.number;
	upd_byte(i-1, kind);
	upd_short(i, CURRENT_PROGRAM_SIZE - i);
	link_start = link_start->l.expr;
    } while (link_start->kind == NODE_BRANCH_LINK);
}

void
i_branch_backwards P2(char, b, int, addr) {
    end_pushes();
    if (b) {
	if (b != F_WHILE_DEC)
	    ins_byte(b);
	ins_short(CURRENT_PROGRAM_SIZE - addr);
    } 
}

static void
i_update_branch_list P1(parse_node_t *, bl) {
    int current_size;
    
    end_pushes();
    current_size = CURRENT_PROGRAM_SIZE;

    while (bl) {
	upd_short(bl->l.number, current_size - bl->l.number);
	bl = bl->v.expr;
    }
}

void
i_generate_else() {
    /* set up a new branch to after the end of the if */
    end_pushes();
    ins_byte(F_BRANCH);
    /* save the old saved value here */
    ins_short(read_short(current_forward_branch));
    /* update the old branch to point to this point */
    upd_short(current_forward_branch, CURRENT_PROGRAM_SIZE - current_forward_branch);
    /* point current_forward_branch at the new branch we made */
    current_forward_branch = CURRENT_PROGRAM_SIZE - 2;
}

void
i_initialize_parser() {
    foreach_depth = 0;
    branch_list[CJ_BREAK] = 0;
    branch_list[CJ_BREAK_SWITCH] = 0;
    branch_list[CJ_CONTINUE] = 0;

    current_forward_branch = 0;

    current_block = A_PROGRAM;
    prog_code = mem_block[A_PROGRAM].block;
    prog_code_max = mem_block[A_PROGRAM].block + mem_block[A_PROGRAM].max_size;

    line_being_generated = 0;
    last_size_generated = 0;
}

void
i_generate_final_program P1(int, x) {
    if (!x) {
	UPDATE_PROGRAM_SIZE;
/* This needs work
 * if (pragmas & PRAGMA_OPTIMIZE)
 *     optimize_icode(0, 0, 0);
 */
	save_file_info(current_file_id, current_line - current_line_saved);
	switch_to_line(-1); /* generate line numbers for the end */
    }
}

/* Currently, this procedure handles:
 * - jump threading
 */
void
optimize_icode P3(char *, start, char *, pc, char *, end) {
    int instr;
    if (start == 0) {
	/* we don't optimize the initializer block right now b/c all the
	 * stuff we do (jump threading, etc) can't occur there.
	 */
	start = mem_block[A_PROGRAM].block;
	pc = start;
	end = pc + mem_block[A_PROGRAM].current_size;
	if (*pc == 0) {
	    /* no initializer jump */
	    pc += 3;
	}
    }
    while (pc < end) {
	switch (instr = EXTRACT_UCHAR(pc++)) {
	case F_NUMBER:
	case F_REAL:
	case F_CALL_INHERITED:
	    pc += 4;
	    break;
	case F_SIMUL_EFUN:
	case F_CALL_FUNCTION_BY_ADDRESS:
	    pc += 3;
	    break;
	case F_BRANCH:
	case F_BRANCH_WHEN_ZERO:
	case F_BRANCH_WHEN_NON_ZERO:
	case F_BBRANCH:
	case F_BBRANCH_WHEN_ZERO:
	case F_BBRANCH_WHEN_NON_ZERO:
	    {
		char *tmp;
		short sarg;
		/* thread jumps */
		COPY_SHORT(&sarg, pc);
		if (instr > F_BRANCH)
		    tmp = pc - sarg;
		else 
		    tmp = pc + sarg;
		sarg = 0;
		while (1) {
		    if (EXTRACT_UCHAR(tmp) == F_BRANCH) {
			COPY_SHORT(&sarg, tmp + 1);
			tmp += sarg + 1;
		    } else if (EXTRACT_UCHAR(tmp) == F_BBRANCH) {
			COPY_SHORT(&sarg, tmp + 1);
			tmp -= sarg - 1;
		    } else break;
		}
		if (!sarg) {
		    pc += 2;
		    break;
		}
		/* be careful; in the process of threading a forward jump
		 * may have changed to a reverse one or vice versa
		 */
		if (tmp > pc) {
		    if (instr > F_BRANCH) {
			pc[-1] -= 3;   /* change to forward branch */
		    }
		    sarg = tmp - pc;
		} else {
		    if (instr <= F_BRANCH) {
			pc[-1] += 3;   /* change to backwards branch */
		    }
		    sarg = pc - tmp;
		}
		STORE_SHORT(pc, sarg);
		break;
	    }
#ifdef F_LOR
	case F_LOR:
	case F_LAND:
	    {
		char *tmp;
		short sarg;
		/* thread jumps */
		COPY_SHORT(&sarg, pc);
		tmp = pc + sarg;
		sarg = 0;
		while (1) {
		    if (EXTRACT_UCHAR(tmp) == F_BRANCH) {
			COPY_SHORT(&sarg, tmp + 1);
			tmp += sarg + 1;
		    } else if (EXTRACT_UCHAR(tmp) == F_BBRANCH) {
			COPY_SHORT(&sarg, tmp + 1);
			tmp -= sarg - 1;
		    } else break;
		}
		if (!sarg) {
		    pc += 2;
		    break;
		}
		/* be careful; in the process of threading a forward jump
		 * may have changed to a reverse one or vice versa
		 */
		if (tmp > pc) {
		    sarg = tmp - pc;
		} else {
#ifdef DEBUG
		    fprintf(stderr,"Optimization failed; can't || or && backwards.\n");
#endif
		    pc += 2;
		    break;
		}
		STORE_SHORT(pc, sarg);
		break;
	    }
#endif
	case F_CATCH:
	case F_AGGREGATE:
	case F_AGGREGATE_ASSOC:
	case F_STRING:
#ifdef F_JUMP_WHEN_ZERO
	case F_JUMP_WHEN_ZERO:
	case F_JUMP_WHEN_NON_ZERO:
#endif
#ifdef F_JUMP
	case F_JUMP:
#endif
	    pc += 2;
	    break;
	case F_GLOBAL_LVALUE:
	case F_GLOBAL:
	case F_SHORT_STRING:
	case F_LOOP_INCR:
	case F_WHILE_DEC:
	case F_LOCAL:
	case F_LOCAL_LVALUE:
	case F_REF:
	case F_REF_LVALUE:
	case F_SSCANF:
	case F_PARSE_COMMAND:
	case F_BYTE:
	case F_NBYTE:
	    pc++;
	    break;
	case F_FUNCTION_CONSTRUCTOR:
	    switch (EXTRACT_UCHAR(pc++)) {
	    case FP_SIMUL:
	    case FP_LOCAL:
		pc += 2;
		break;
	    case FP_FUNCTIONAL:
	    case FP_FUNCTIONAL | FP_NOT_BINDABLE:
		pc += 3;
		break;
	    case FP_ANONYMOUS:
	    case FP_ANONYMOUS | FP_NOT_BINDABLE:
		pc += 4;
		break;
	    case FP_EFUN:
		pc += 2;
		break;
	    }
	    break;
	case F_SWITCH:
	    {
		unsigned short stable, etable;
		pc++; /* table type */
		LOAD_SHORT(stable, pc);
		LOAD_SHORT(etable, pc);
		pc += 2; /* def */
		DEBUG_CHECK(stable < pc - start || etable < pc - start 
			    || etable < stable,
			    "Error in switch table found while optimizing\n");
		/* recursively optimize the inside of the switch */
		optimize_icode(start, pc, start + stable);
		pc = start + etable;
		break;
	    }
	case F_EFUN0:
	case F_EFUN1:
	case F_EFUN2:
	case F_EFUN3:
	case F_EFUNV:
	    instr = EXTRACT_UCHAR(pc++) + ONEARG_MAX;
	default:
	    if ((instr >= BASE) && 
		(instrs[instr].min_arg != instrs[instr].max_arg))
		pc++;
	}
    }
}
	    
		



