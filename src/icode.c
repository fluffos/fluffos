/* 
 * code generator for runtime LPC code
 * 
 * Currently only generates code for expressions
 */
#include "std.h"
#include "icode.h"
#include "trees.h"
#include "compiler.h"
#include "include/origin.h"
#include "lex.h"
#include "interpret.h"
#include "generate.h"
#include "simulate.h"

/* These should be static as soon as the switch generation code moves in
   here */
static void ins_real PROT((double));
void ins_short PROT((short));
void upd_short PROT((int, short));
static void ins_byte PROT((unsigned char));
static void upd_byte PROT((int, unsigned char));
INLINE void ins_f_byte PROT((unsigned int));
static void write_number PROT((int));
short read_short PROT((int));
void ins_long PROT((int));
void push_address PROT((void));
int pop_address PROT((void));
void push_explicit PROT((int));
void i_generate_node PROT((struct parse_node *));

/* These should be static too */
/*
   these three variables used to properly adjust the 'break_sp' stack in
   the event a 'continue' statement is issued from inside a 'switch'.
*/
short switches = 0;
int switch_sptr = 0;

int current_break_address;
int current_continue_address;
int current_forward_branch;
int current_case_number_heap;
int current_case_string_heap;
int zero_case_label;

static int last_size_generated;
static int line_being_generated;

static int comp_stackp;
static int comp_stack[COMPILER_STACK_SIZE];

void push_explicit P1(int, address)
{
    if (comp_stackp >= COMPILER_STACK_SIZE) {
	yyerror("Compiler stack overflow");
	comp_stackp++;
	return;
    }
    comp_stack[comp_stackp++] = address;
}

void push_address()
{
    if (comp_stackp >= COMPILER_STACK_SIZE) {
	yyerror("Compiler stack overflow");
	comp_stackp++;
	return;
    }
    comp_stack[comp_stackp++] = mem_block[current_block].current_size;
}

int pop_address()
{
    if (comp_stackp == 0)
	fatal("Compiler stack underflow.\n");
    if (comp_stackp > COMPILER_STACK_SIZE) {
	--comp_stackp;
	return 0;
    }
    return comp_stack[--comp_stackp];
}

static void ins_real P1(double, l)
{
    float f = (float)l;

    struct mem_block *mbp = &mem_block[current_block];
    if (mbp->current_size + 4 > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + 4);
    mbp->block[mbp->current_size++] = ((char *)&f)[0];
    mbp->block[mbp->current_size++] = ((char *)&f)[1];
    mbp->block[mbp->current_size++] = ((char *)&f)[2];
    mbp->block[mbp->current_size++] = ((char *)&f)[3];
}

/*
 * Store a 2 byte number. It is stored in such a way as to be sure
 * that correct byte order is used, regardless of machine architecture.
 * Also beware that some machines can't write a word to odd addresses.
 */
void ins_short P1(short, l)
{
    struct mem_block *mbp = &mem_block[current_block];
    if (mbp->current_size + 2 > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + 2);
    mbp->block[mbp->current_size++] = ((char *)&l)[0];
    mbp->block[mbp->current_size++] = ((char *)&l)[1];
}

short read_short P1(int, offset)
{
    short l;

    ((char *)&l)[0] = mem_block[current_block].block[offset];
    ((char *)&l)[1] = mem_block[current_block].block[offset + 1];
    return l;
}

/*
 * Store a 4 byte number. It is stored in such a way as to be sure
 * that correct byte order is used, regardless of machine architecture.
 */
void ins_long P1(int, l)
{
    struct mem_block *mbp = &mem_block[current_block];
    if (mbp->current_size + 4 > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + 4);
    mbp->block[mbp->current_size++] = ((char *)&l)[0];
    mbp->block[mbp->current_size++] = ((char *)&l)[1];
    mbp->block[mbp->current_size++] = ((char *)&l)[2];
    mbp->block[mbp->current_size++] = ((char *)&l)[3];
}

void upd_short P2(int, offset, short, l)
{
    DEBUG_CHECK2(offset > CURRENT_PROGRAM_SIZE,
		 "patch offset %x larger than current program size %x.\n",
		 offset, CURRENT_PROGRAM_SIZE);
    mem_block[current_block].block[offset] = ((char *)&l)[0];
    mem_block[current_block].block[offset + 1] = ((char *)&l)[1];
}

static void ins_byte P1(unsigned char, b)
{
    struct mem_block *mbp = &mem_block[current_block];
    if (mbp->current_size == mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + 1);
    mbp->block[mbp->current_size++] = b;
}

static void upd_byte P2(int, offset, unsigned char, b)
{
    DEBUG_CHECK2(offset > CURRENT_PROGRAM_SIZE,
		"patch offset %x larger than current program size %x.\n",
		offset, CURRENT_PROGRAM_SIZE);
    mem_block[current_block].block[offset] = b;
}

INLINE
void ins_f_byte P1(unsigned int, b)
{
#ifdef NEEDS_CALL_EXTRA
    if (b >= 0xff) {
	ins_byte((char)F_CALL_EXTRA);
	ins_byte((char)(b - 0xff));
    } else {
#endif
	ins_byte((char)b);
#ifdef NEEDS_CALL_EXTRA
    }
#endif
}

/*
 * Generate the code to push a number on the stack.
 * This varies since there are several opcodes (for
 * optimizing speed and/or size).
 */
static void write_number P1(int, val)
{
    if (val == 0) {
	ins_f_byte(F_CONST0);
    } else if (val == 1) {
	ins_f_byte(F_CONST1);
    } else if (val > 0 && val < 256) {
	ins_f_byte(F_BYTE);
	ins_byte(val);
    } else if (val < 0 && val > -256) {
	ins_f_byte(F_NBYTE);
	ins_byte(-val);
    } else {
	ins_f_byte(F_NUMBER);
	ins_long(val);
    }
}

static void
generate_expr_list P1(struct parse_node *, expr) {
    if (!expr) return;
    do {
      i_generate_node(expr->v.expr);
    } while (expr = expr->right);
}

static void
generate_lvalue_list P1(struct parse_node *, expr) {
    while (expr = expr->right) {
      i_generate_node(expr->left);
      ins_f_byte(F_VOID_ASSIGN);
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
	    *p++ = ((char *)&s)[0];
	    *p = ((char *)&s)[1];
	    sz -= 255;
	}
	p = (unsigned char *)allocate_in_mem_block(A_LINENUMBERS, 3);
	*p++ = sz;
	*p++ = ((char *)&s)[0];
	*p = ((char *)&s)[1];
    }
    line_being_generated = line;
}

void
i_generate_node P1(struct parse_node *, expr) {
    if (!expr) return;

    if (expr->line && expr->line != line_being_generated)
	switch_to_line(expr->line);
    switch (expr->kind) {
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
	i_generate_node(expr->left);
	/* fall through */
    case F_RETURN:
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
	i_generate_node(expr->right);
	/* fall through */
    case F_CONST0:
    case F_CONST1:
    case F_BREAK_POINT:
    case F_BREAK:
	ins_f_byte(expr->kind);
	break;
    case F_ASSIGN: /* note these are backwards */
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
	i_generate_node(expr->right);
	i_generate_node(expr->left);
	ins_f_byte(expr->kind);
	break;
    case F_RANGE:
	i_generate_node(expr->v.expr);
	i_generate_node(expr->left);
	i_generate_node(expr->right);
	ins_f_byte(F_RANGE);
	break;
    case F_STRING:
	ins_f_byte(expr->kind);
	ins_short(expr->v.number);
	break;
    case F_REAL:
	ins_f_byte(F_REAL);
	ins_real(expr->v.real);
	break;
    case F_NBYTE:
    case F_BYTE:
	ins_f_byte(expr->kind);
	ins_byte(expr->v.number);
	break;
    case F_NUMBER:
	write_number(expr->v.number);
	break;
    case F_LOR:
    case F_LAND:
	i_generate_node(expr->left);
	i_generate_forward_branch(expr->kind);
	i_generate_node(expr->right);
	i_update_forward_branch();
	break;
    case F_AGGREGATE:
    case F_AGGREGATE_ASSOC:
	generate_expr_list(expr->right);
	ins_f_byte(expr->kind);
	ins_short(expr->v.number);
	break;
    case NODE_COMMA:
    case NODE_ASSOC:
	i_generate_node(expr->left);
	i_generate_node(expr->right);
	break;
    case NODE_BREAK:
	ins_f_byte(F_BRANCH);
	ins_short(current_break_address);
	current_break_address = CURRENT_PROGRAM_SIZE -2;
	break;
    case NODE_CONTINUE:
	if (switches) {
	    ins_f_byte(F_POP_BREAK);
	    ins_byte(switches);
	}
	ins_f_byte(F_BRANCH);
	ins_short(current_continue_address);
	current_continue_address = CURRENT_PROGRAM_SIZE - 2;
	break;
    case NODE_STATEMENTS:
	i_generate_node(expr->left);
	i_generate_node(expr->right);
	break;
    case NODE_IF:
	if (expr->v.expr->kind == F_NOT) {
	    i_generate_node(expr->v.expr->right);
	    i_generate_forward_branch(F_BRANCH_WHEN_NON_ZERO);
	} else {
	    i_generate_node(expr->v.expr);
	    i_generate_forward_branch(F_BRANCH_WHEN_ZERO);
	}
	i_generate_node(expr->left);
	if (expr->right) {
	    i_generate_else();
	    i_generate_node(expr->right);
	}
	i_update_forward_branch();
	break;
    case NODE_FOR:
	{
	    int forever = node_always_true(expr->left->v.expr);
	    
	    i_save_loop_info();
	    i_generate_node(expr->left->left);
	    if (!forever) 
		i_generate_forward_branch(F_BRANCH);
	    i_save_position();
	    i_generate_node(expr->right);
	    i_update_continues();
	    i_generate_node(expr->left->right);
	    if (!forever)
		i_update_forward_branch();
	    generate_conditional_branch(expr->left->v.expr);
	    i_update_breaks();
	    i_restore_loop_info();
	}
	break;
    case NODE_WHILE:
	{
	    int forever = node_always_true(expr->left);
	    i_save_loop_info();
	    if (!forever)
		i_generate_forward_branch(F_BRANCH);
	    i_save_position();
	    
	    i_generate_node(expr->right);
	    if (!forever)
		i_update_forward_branch();
	    i_update_continues();
	    generate_conditional_branch(expr->left);
	    i_update_breaks();
	    i_restore_loop_info();
	}
	break;
    case NODE_CASE_NUMBER:
    case NODE_CASE_STRING:
	if (expr->v.expr) {
	    struct parse_node *other = expr->v.expr;
	    expr->v.number = 1;
	    other->left = expr->left;
	    other->v.number = CURRENT_PROGRAM_SIZE;
	    expr->left = other;
	} else {
	    expr->v.number = CURRENT_PROGRAM_SIZE;
	}
      break;
    case NODE_DEFAULT:
	expr->v.number = CURRENT_PROGRAM_SIZE;
	break;
    case NODE_SWITCH_STRINGS:
    case NODE_SWITCH_NUMBERS:
    case NODE_SWITCH_DIRECT:
	{
	    int addr;

	    i_generate_node(expr->left);
	    switches++;
	    ins_f_byte(F_SWITCH);
	    ins_byte(0xff); /* kind of table */
	    addr = CURRENT_PROGRAM_SIZE;
	    ins_short(0); /* address of table */
	    ins_short(0); /* break address to push, table is entered before */
	    ins_short(0); /* default address */
	    i_generate_node(expr->right);
	    if (expr->v.expr && expr->v.expr->kind == NODE_DEFAULT) {
		upd_short(addr + 4, expr->v.expr->v.number);
		expr->v.expr = expr->v.expr->left;
	    } else {
		upd_short(addr + 4, CURRENT_PROGRAM_SIZE);
	    }
	    /* just in case the last case doesn't have a break */
	    ins_f_byte(F_BREAK);
	    /* build table */
	    upd_short(addr, CURRENT_PROGRAM_SIZE);
#ifdef SAVE_BINARIES
	    if (expr->kind == NODE_SWITCH_STRINGS) {
		short sw;
		sw = addr - 2;
		add_to_mem_block(A_PATCH, (char *)&sw, sizeof sw);
	    }
#endif
	    if (expr->kind == NODE_SWITCH_DIRECT) {
		struct parse_node *pn = expr->v.expr;
		while (pn) {
		    ins_short((short)pn->v.number);
		    pn = pn->left;
		}
		ins_long((int)expr->v.expr->right);
		mem_block[current_block].block[addr-1] = (char)0xfe;
	    } else {
		int table_size = 0;
		int power_of_two = 1;
		int i = 0;
		struct parse_node *pn = expr->v.expr;
		
		while (pn) {
		    ins_long((int)pn->right);
		    ins_short((short)pn->v.number);
		    pn = pn->left;
		    table_size += 1;
		}
		while ((power_of_two<<1) <= table_size) {
		    power_of_two <<= 1;
		    i++;
		}
		if (expr->kind == NODE_SWITCH_NUMBERS)
		    mem_block[current_block].block[addr-1] = (char)(0xf0+i);
		else
		    mem_block[current_block].block[addr-1] = (char)(i*0x10+0x0f);
	    }
	    upd_short(addr + 2, CURRENT_PROGRAM_SIZE);
	    break;
	}
    case NODE_CONDITIONAL:
	{
	    int addr;
	    
	    i_generate_node(expr->left);
	    ins_f_byte(F_BRANCH_WHEN_ZERO);
	    addr = CURRENT_PROGRAM_SIZE;
	    ins_short(0);
	    
	    i_generate_node(expr->right->left);
	    upd_short(addr, CURRENT_PROGRAM_SIZE - addr + 3); /*over the branch */
	    ins_f_byte(F_BRANCH);
	    addr = CURRENT_PROGRAM_SIZE;
	    ins_short(0);
	    
	    i_generate_node(expr->right->right);
	    upd_short(addr, CURRENT_PROGRAM_SIZE - addr);
	}
	break;
    case F_CATCH:
	{
	    int addr;
	    
	    ins_f_byte(F_CATCH);
	    addr = CURRENT_PROGRAM_SIZE;
	    ins_short(0);
	    i_generate_node(expr->right);
	    ins_f_byte(F_END_CATCH);
	    upd_short(addr, CURRENT_PROGRAM_SIZE - addr);
	    break;
	}
    case F_SSCANF:
	i_generate_node(expr->left->left);
	i_generate_node(expr->left->right);
	ins_f_byte(F_SSCANF);
	ins_byte(expr->right->v.number);
	generate_lvalue_list(expr->right);
	break;
    case F_PARSE_COMMAND:
	i_generate_node(expr->left->left);
	i_generate_node(expr->left->right->left);
	i_generate_node(expr->left->right->right);
	ins_f_byte(F_PARSE_COMMAND);
	ins_byte(expr->right->v.number);
	generate_lvalue_list(expr->right);
	break;
    case F_TIME_EXPRESSION:
	ins_f_byte(F_TIME_EXPRESSION);
	i_generate_node(expr->right);
	ins_f_byte(F_END_TIME_EXPRESSION);
	break;
    case F_TO_FLOAT:
    case F_TO_INT:
	generate_expr_list(expr->right);
	ins_f_byte(expr->kind);
	break;
    case F_GLOBAL_LVALUE:
    case F_GLOBAL:
    case F_LOCAL_LVALUE:
    case F_LOCAL:
    case F_LOOP_INCR:
    case F_WHILE_DEC:
	ins_f_byte(expr->kind);
	ins_byte(expr->v.number);
	break;
    case F_LOOP_COND:
	{
	    int i;
	    
	    ins_f_byte(F_LOOP_COND);
	    ins_byte(expr->left->v.number);
	    /* expand this into a number so we can pull it fast at runtime */
	    if (expr->right->kind == F_LOCAL) {
		i_generate_node(expr->right);
	    } else {
		ins_f_byte(F_NUMBER);
		switch (expr->right->kind) {
		case F_CONST0: i = 0; break;
		case F_CONST1: i = 1; break;
		case F_NBYTE: i = - expr->right->v.number; break;
		case F_BYTE:
		case F_NUMBER:
		    i = expr->right->v.number; break;
		default:
		    fatal("Unknown node %i in F_LOOP_COND\n", expr->right->kind);
		}
		ins_long(i);
	    }
	    ins_f_byte(F_LT);
	    break;
	}
    case F_SIMUL_EFUN:
    case F_CALL_FUNCTION_BY_ADDRESS:
	generate_expr_list(expr->right);
	ins_f_byte(expr->kind);
	ins_short(expr->v.number);
	ins_byte((expr->right ? expr->right->kind : 0));
	break;
    case F_EVALUATE:
#ifdef NEW_FUNCTIONS
	generate_expr_list(expr->right);
	ins_f_byte(F_EVALUATE);
#else
	i_generate_node(expr->left);
	ins_f_byte(F_EVALUATE);
	generate_expr_list(expr->right);
	ins_f_byte(F_CALL_OTHER);
#endif
	ins_byte(expr->v.number);
	break;
    case F_FUNCTION_CONSTRUCTOR:
#ifdef NEW_FUNCTIONS
	if ((expr->v.number & 0xff) == FP_CALL_OTHER) {
	    i_generate_node(expr->left);
	    i_generate_node(expr->right);
	    ins_f_byte(F_FUNCTION_CONSTRUCTOR);
	    ins_f_byte(ORIGIN_CALL_OTHER);
	    break;
	}
	if (expr->right) {
	    generate_expr_list(expr->right);
	    ins_f_byte(F_AGGREGATE);
	    ins_short(expr->right->kind);
	} else if ((expr->v.number & 0xff) != FP_FUNCTIONAL)
	    ins_f_byte(F_CONST0);
	ins_f_byte(F_FUNCTION_CONSTRUCTOR);
	switch (expr->v.number & 0xff) {
	case FP_SIMUL_EFUN:
	    ins_byte(ORIGIN_SIMUL_EFUN);
	    ins_short((expr->v.number & ~0xff) >> 8);
	    break;
	case FP_EFUN:
	    ins_byte(ORIGIN_EFUN);
	    ins_f_byte(predefs[(expr->v.number & ~0xff) >> 8].token);
	break;
	case FP_LFUN:
	    ins_byte(ORIGIN_LOCAL);
	    ins_short((expr->v.number & ~0xff) >> 8);
	    break;
	case FP_FUNCTIONAL:
	    {
		int addr;
		ins_byte(ORIGIN_FUNCTIONAL);
		ins_byte((expr->v.number & ~0xff) >> 8);
		addr = CURRENT_PROGRAM_SIZE;
		ins_short(0);
		i_generate_node(expr->left);
		ins_f_byte(F_RETURN);
		upd_short(addr, CURRENT_PROGRAM_SIZE - addr - 2);
	    }
	}
#else
	if (expr->left) i_generate_node(expr->left);
	i_generate_node(expr->right);
	ins_f_byte(F_FUNCTION_CONSTRUCTOR);
	ins_byte(expr->v.number);
#endif
	break;
    default:
	DEBUG_CHECK1(expr->kind < BASE,
		     "Unknown eoperator %s in i_generate_node.\n",
		     get_f_name(expr->kind));
	generate_expr_list(expr->right);
	ins_f_byte(expr->kind);
	if (expr->v.number != -1)
	    ins_byte(expr->v.number);
    }
}

void
i_generate_function_call P2(short, f, char, num) {
  ins_f_byte(F_CALL_FUNCTION_BY_ADDRESS);
  ins_short(f);
  ins_byte(0);
}

void
i_pop_value() {
  ins_f_byte(F_POP_VALUE);
}

void i_generate_return P1(struct parse_node *, node) {
  if (node) {
    i_generate_node(node);
  } else {
    ins_f_byte(F_CONST0);
  }
  ins_f_byte(F_RETURN);
}

void i_generate___INIT() {
    current_block = A_PROGRAM;
    upd_byte(0, F_BRANCH);
    upd_short(1, CURRENT_PROGRAM_SIZE - 1);
    add_to_mem_block(A_PROGRAM, (char *)mem_block[A_INITIALIZER].block,
		     mem_block[A_INITIALIZER].current_size);
}

void i_generate_continue() {
  if (switches) {
    ins_f_byte(F_POP_BREAK);
    ins_byte(switches);
  }
  /* form a linked list of the continue addresses */
  ins_f_byte(F_BRANCH);
  ins_short(current_continue_address);
  current_continue_address = CURRENT_PROGRAM_SIZE - 2;
}

void i_generate_forward_branch P1(char, b) {
  ins_f_byte(b);
  ins_short(current_forward_branch);
  current_forward_branch = CURRENT_PROGRAM_SIZE - 2;
}

void
i_update_forward_branch() {
  int i = read_short(current_forward_branch);
  upd_short(current_forward_branch, CURRENT_PROGRAM_SIZE - current_forward_branch);
  current_forward_branch = i;
}

void
i_save_position() {
  push_address();
}

void
i_branch_backwards P1(char, b) {
    if (b) {
	ins_f_byte(b);
	ins_short(CURRENT_PROGRAM_SIZE - pop_address());
    } else pop_address();
}

void
i_update_breaks() {
  int next_addr;

  /* traverse the linked list filling in the current_break_address
     required by each "break" statement.
     */
  for (;current_break_address > 0; current_break_address = next_addr) {
    next_addr = read_short(current_break_address);
    upd_short(current_break_address, CURRENT_PROGRAM_SIZE - current_break_address);
  }
}

void
i_update_continues() {
  int next_addr;

  /* traverse the linked list filling in the current_continue_address
     required by each "continue" statement.
     */
  for (;current_continue_address > 0; current_continue_address = next_addr) {
    next_addr = read_short(current_continue_address);
    upd_short(current_continue_address, CURRENT_PROGRAM_SIZE - current_continue_address);
  }
}

void
i_save_loop_info() {
  /* each 'continue' statement will add to a linked list of slots that
     need filled in once the continue_address is known.  The delimeters
     serve as markers for the ends of the lists.
     */
  push_explicit(current_continue_address);
  current_continue_address = 0;
  push_explicit(current_break_address);
  current_break_address = 0;
  /* keep track of # of nested switches prior to this while
     so that we'll know how many to pop from the break stack in
     the event of a "continue;" in the body of a switch().
     */
  push_explicit(switches);
  switches = 0;
}

void
i_restore_loop_info() {
    switches = pop_address();
    current_break_address = pop_address();
    current_continue_address = pop_address();
}

void
i_generate_else() {
  /* set up a new branch to after the end of the if */
  ins_f_byte(F_BRANCH);
  /* save the old saved value here */
  ins_short(read_short(current_forward_branch));
  /* update the old branch to point to this point */
  upd_short(current_forward_branch, CURRENT_PROGRAM_SIZE - current_forward_branch);
  /* point current_forward_branch at the new branch we made */
  current_forward_branch = CURRENT_PROGRAM_SIZE - 2;
}

void
i_initialize_parser() {
    short s = 0;
    unsigned char *p;

    switches = 0;
    switch_sptr = 0;
    comp_stackp = 0;
    current_continue_address = 0;
    current_break_address = 0;
    current_forward_branch = 0;
    current_block = A_PROGRAM;
    ins_byte(0); /* will be changed to an F_BRANCH if an initializer
		    exists */
    ins_short(0);
    
    line_being_generated = 0;
    last_size_generated = 3;
    p = (unsigned char *)allocate_in_mem_block(A_LINENUMBERS, 3);
    *p++ = 3;
    *p++ = ((char *)&s)[0];
    *p = ((char *)&s)[1];
}

void
i_generate_final_program P1(int, x) {
    if (!x) {
	if (pragmas & PRAGMA_OPTIMIZE)
	    optimize_icode(0, 0, 0);
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
		((char *) &sarg)[0] = pc[0];
		((char *) &sarg)[1] = pc[1];
		if (instr > F_BRANCH)
		    tmp = pc - sarg;
		else 
		    tmp = pc + sarg;
		sarg = 0;
		while (1) {
		    if (EXTRACT_UCHAR(tmp) == F_BRANCH) {
			((char *) &sarg)[0] = tmp[1];
			((char *) &sarg)[1] = tmp[2];
			tmp += sarg + 1;
		    } else if (EXTRACT_UCHAR(tmp) == F_BBRANCH) {
			((char *) &sarg)[0] = tmp[1];
			((char *) &sarg)[1] = tmp[2];
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
		*pc++ = ((char *) &sarg)[0];
		*pc++ = ((char *) &sarg)[1];
		break;
	    }
#ifdef F_LOR
	case F_LOR:
	case F_LAND:
	    {
		char *tmp;
		short sarg;
		/* thread jumps */
		((char *) &sarg)[0] = pc[0];
		((char *) &sarg)[1] = pc[1];
		tmp = pc + sarg;
		sarg = 0;
		while (1) {
		    if (EXTRACT_UCHAR(tmp) == F_BRANCH) {
			((char *) &sarg)[0] = tmp[1];
			((char *) &sarg)[1] = tmp[2];
			tmp += sarg + 1;
		    } else if (EXTRACT_UCHAR(tmp) == F_BBRANCH) {
			((char *) &sarg)[0] = tmp[1];
			((char *) &sarg)[1] = tmp[2];
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
		    break;
		}
		*pc++ = ((char *) &sarg)[0];
		*pc++ = ((char *) &sarg)[1];
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
	case F_LOOP_INCR:
	case F_WHILE_DEC:
	case F_LOOP_COND:
	case F_LOCAL:
	case F_LOCAL_LVALUE:
	case F_SSCANF:
	case F_PARSE_COMMAND:
	case F_BYTE:
	case F_POP_BREAK:
	case F_NBYTE:
	    pc++;
	    break;
	case F_FUNCTION_CONSTRUCTOR:
	    switch (EXTRACT_UCHAR(pc++)) {
	    case ORIGIN_SIMUL_EFUN:
	    case ORIGIN_LOCAL:
		pc += 2;
		break;
	    case ORIGIN_FUNCTIONAL:
		pc += 3;
		break;
	    case ORIGIN_EFUN:
#ifdef NEEDS_CALL_EXTRA
		if (EXTRACT_UCHAR(pc++) == F_CALL_EXTRA)
#endif
		    pc++;
		break;
	    }
	    break;
	case F_SWITCH:
	    {
		unsigned short stable, etable;
		pc++; /* table type */
		((char *) &stable)[0] = EXTRACT_UCHAR(pc++);
		((char *) &stable)[1] = EXTRACT_UCHAR(pc++);
		((char *) &etable)[0] = EXTRACT_UCHAR(pc++);
		((char *) &etable)[1] = EXTRACT_UCHAR(pc++);
		pc += 2; /* def */
		DEBUG_CHECK(stable < pc - start || etable < pc - start 
			    || etable < stable,
			    "Error in switch table found while optimizing\n");
		/* recursively optimize the inside of the switch */
		optimize_icode(start, pc, start + stable);
		pc = start + etable;
		break;
	    }
	case F_CALL_EXTRA:
	    instr = EXTRACT_UCHAR(pc++) + 0xff;
	default:
	    if ((instr >= BASE) && 
		(instrs[instr].min_arg != instrs[instr].max_arg))
		pc++;
	}
    }
}

