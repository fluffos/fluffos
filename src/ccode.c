/* 
 * code generator for LPC->C compilation
 * 
 * Currently only generates code for expressions
 */
#ifndef CCODE_H
#define CCODE_H
#include "std.h"
#include "trees.h"

void c_generate_node PROT((struct parse_node *));

static void
generate_expr_list P1(struct parse_node *, expr) {
    if (!expr) return;
    do {
      c_generate_node(expr->v.expr);
    } while (expr = expr->right);
}

static void
generate_efun_expr_list P2(struct parse_node *, expr, int, keep_in_reg) {
    if (!expr) return;
    do {
      c_generate_node(expr->v.expr);
      if (keep_in_reg)
	  keep_in_reg--;
      else
	  change_destination_to_stack();
    } while (expr = expr->right);
}

void
i_generate_final_program P1(int, x) {
    if (!x) {
	ins_string("int (*__FUNCS");
	if (compilation_ident) {
	    ins_string("_");
	    ins_string(compilation_ident);
	}
	ins_string("[])() = {\n");
    } else {
	for (i = 0; i < mem_block[A_FUNCTIONS].current_size; i += sizeof *funp) {
	    funp = (struct function *)(mem_block[A_FUNCTIONS].block + i);
	    if (!(funp->flags & NAME_UNDEFINED) &&
		!(funp->flags & NAME_INHERITED)) {
		ins_string("_lpc_");
		ins_string(funp->name);
		ins_string(",\n");
	    } else {
		ins_string("0,\n");
	    }
	}
	ins_string("};\n");
	fwrite(mem_block[A_PROGRAM].block, mem_block[A_PROGRAM].current_size,
	       1, compilation_output_file);
    }
}

static void
generate_expr_list_on_stack P1(struct parse_node *, expr) {
    if (!expr) return;
    do {
      c_generate_node(expr->v.expr);
      change_destination_to_stack();
    } while (expr = expr->right);
}

static void
generate_lvalue_list P1(struct parse_node *, expr) {
    int i;

    while (expr = expr->right) {
      c_generate_node(expr->left);
      ins_cfun("C_ASSIGN_FROM_STACK", 1);
    }
}

void
c_generate_node P1(struct parse_node *, expr) {
    if (!expr) return;

    switch (expr->kind) {
    case F_CONST0:
	stack_push_number(0);
	break;
    case F_CONST1:
	stack_push_number(1);
	break;
    case F_POP_VALUE:
	stack_pop();
	break;
    case F_PRE_INC:
    case F_PRE_DEC:
    case F_INC:
    case F_DEC:
    case F_POST_INC:
    case F_POST_DEC:
    case F_NOT:
    case F_COMPL:
    case F_NEGATE:
	c_generate_node(expr->right);
	ins_cfun_call(expr->kind, 1);
	break;
    case F_INDEXED_LVALUE:
    case F_INDEX:
	if(needs_comma) {
#if defined(LPC_TRACE) || defined(LPC_DEBUG)
	    prefab_in_progress = 1;
#endif
            do_comma();
            create_intermediates(2);
            BACKSPACE(2);
#if defined(LPC_TRACE) || defined(LPC_DEBUG)
	    prefab_in_progress = 0;
#endif
	} else
	    create_intermediates(2);

	/* save a register for use a temporary svalue for string[] and
	   buffer[] */
	stack_push_register(get_register(), 0);
	
	regs_saved = registers_in_use;
	strcpy(tmp,"c_index(");
	/* prepare_arguments marks the registers as unused, but they are
	 * still used indirectly in the prefab
	 */
	prepare_arguments(tmp+8,3,0,0);
	strcat(tmp,")");
	stack_push_prefab(tmp, registers_in_use);
	/* put back to state it was in before the prepare_arguments */
	registers_in_use = regs_saved;
	generate_frees();
	break;
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
      c_generate_node(expr->left);
      c_generate_node(expr->right);
      ins_cfun_call(expr->kind, 2);
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
	c_generate_node(expr->right);
	c_generate_node(expr->left);
	ins_cfun_call(expr->kind, 2);
	break;
    case F_RANGE:
	c_generate_node(expr->v.expr);
	c_generate_node(expr->left);
	c_generate_node(expr->right);
	ins_cfun_call(F_RANGE, 3);
	generate_frees();
	break;
    case F_STRING:
	stack_push_string(expr->v.number);
	break;
    case F_REAL:
	stack_push_real($1);
	break;
    case F_NBYTE:
	stack_push_number(-(expr->v.number));
	break;
    case F_BYTE:
	stack_push_number(expr->v.number);
	break;
    case F_NUMBER:
	stack_push_number(expr->v.number);
	break;
    case F_LOR:
    case F_LAND:
	c_generate_node(expr->left);
	must_have_register();
	generate_frees();
	do_comma();
	ins_string("\n(");
	needs_comma = 0;
	if (expr->kind == F_LOR)
	    ins_cfun("C_IS_FALSE", 1);
	else
	    ins_cfun("C_IS_TRUE", 1);
	ins_string(" ?\n");
	needs_comma = 0;
	c_generate_node(expr->right);
	must_have_register();
	generate_frees();
	ins_string(":0)\n");
	break;
    case F_AGGREGATE:
	generate_expr_list_on_stack(expr->right);
	ins_ext_cfun("C_AGGREGATE", 0, expr->v.number);
	generate_frees();
	break;
    case F_AGGREGATE_ASSOC:
	generate_expr_list_on_stack(expr->right);
	ins_ext_cfun("C_ASSOC", 0, expr->v.number);
	generate_frees();
	break;
    case NODE_ASSOC:
	c_generate_node(expr->left);
	change_destination_to_stack();
	c_generate_node(expr->right);
	change_destination_to_stack();
	break;
    case NODE_COMMA:
	c_generate_node(expr->left);
	c_generate_node(expr->right);
	break;
    case NODE_CONDITIONAL:
	{
	    int reg;
	    int needs_free;
	    
	    c_generate_node(expr->left);
	    generate_truth_test();
	    ins_string("\n? (\n");
	    needs_comma = 0;
	    
	    c_generate_node(expr->right->left);
	    must_have_register();
	    /* save what register we're using so we can use the same one
	       below */
	    reg = TOP->u.num;
	    if (TOP_NEEDS_FREE) {
		needs_free = 1;
		MARK_NEEDS_NO_FREE(reg);
	    } else
		needs_free = 0;
	    generate_frees();
	    stack_pop();
	    ins_string("\n) : (\n");
	    needs_comma = 0;
	    
	    c_generate_node(expr->right->right);
	    must_have_register();
	    if (TOP->u.num != reg) {
		/* move to the correct register */
		char buf[100];
		if (TOP_NEEDS_FREE) needs_free = 1;
		do_comma();
		sprintf(buf, "r%i=r%i", reg, TOP->u.num);
		ins_string(buf);
		stack_pop();
		stack_push_register(reg, needs_free);
	    } else if (needs_free) 
		MARK_NEEDS_FREE(reg);
	    ins_string("\n)\n");
	}
	break;
    case F_CATCH:
      {
	int reg = get_register();

	stack_push_register(reg, 1);
	do_comma();
	ins_string("CATCH_START,\n(SETJMP(error_recovery_context) ?");
	ins_string_with_num("(CATCH_ERROR,\nr%i = catch_value,\n", reg);
	ins_string("catch_value = const1) :\n(");
	c_generate_node(insert_pop_value(expr->right));
	ins_string(",\n");
	/* top of the stack is the register we pushed before */
	ins_string_with_num("CATCH_END, r%i = const0))\n", TOP->u.num);
	needs_comma = 1;
	break;
      }
    case F_SSCANF:
	c_generate_node(expr->left->left);
	c_generate_node(expr->left->right);
	ins_ext_cfun("c_sscanf", 2, expr->right->v.number);
	generate_frees();
	generate_lvalue_list(expr->right);
	break;
    case F_PARSE_COMMAND:
	c_generate_node(expr->left->left);
	c_generate_node(expr->left->right->left);
	c_generate_node(expr->left->right->right);
	ins_ext_cfun("c_parse_command", 3, expr->right->v.number);
	generate_frees();
	generate_lvalue_list(expr->right);
	break;
    case F_TIME_EXPRESSION:
	ins_cfun("C_TIME_EXPRESSION", 0);
	c_generate_node(expr->right);
	ins_cfun_call_by_name("C_END_TIME_EXPRESSION", 1, 0);
	generate_frees();
	break;
    case F_TO_FLOAT:
    case F_TO_INT:
	generate_expr_list(expr->right);
	ins_cfun_call(expr->kind);
	break;
    case F_GLOBAL_LVALUE:
    case F_GLOBAL:
	stack_push_identifier(expr->v.number);
	break;
    case F_LOCAL_LVALUE:
    case F_LOCAL:
	stack_push_local(expr->v.number);
	break;
    case F_LOOP_INCR:
    case F_WHILE_DEC:
	stack_push_local(expr->v.number);
	ins_cfun_call(expr->v.number);
	break;
    case F_LOOP_COND:
	ins_cfun_call(F_LOOP_COND);
	break;
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
      c_generate_node(expr->left);
      ins_f_byte(F_EVALUATE);
      c_generate_expr_list(expr->right);
      ins_f_byte(F_CALL_OTHER);
#endif
      ins_byte(expr->v.number);
      break;
    case F_FUNCTION_CONSTRUCTOR:
#ifdef NEW_FUNCTIONS
      if ((expr->v.number & 0xff) == FP_CALL_OTHER) {
	c_generate_node(expr->left);
	c_generate_node(expr->right);
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
	  c_generate_node(expr->left);
	  ins_f_byte(F_RETURN);
	  upd_short(addr, CURRENT_PROGRAM_SIZE - addr + 2);
	}
      }
      break;
#else
      if (expr->left) c_generate_node(expr->left);
      c_generate_node(expr->right);
      ins_f_byte(F_FUNCTION_CONSTRUCTOR);
      ins_byte(expr->v.number);
#endif
    default:
	DEBUG_CHECK1(expr->kind < BASE,
		     "Unknown eoperator %s in c_generate_node.\n", 
		     get_f_name(expr->kind));
	generate_expr_list(expr->right);
	ins_f_byte(expr->kind);
	if (expr->v.number != -1)
	    ins_byte(expr->v.number);
    }
}

void
c_generate_function_call P2(short, f, char, num) {
  ins_f_byte(F_CALL_FUNCTION_BY_ADDRESS);
  ins_short(f);
  ins_byte(0);
}

void
c_pop_value() {
  ins_f_byte(F_POP_VALUE);
}

void c_generate_return P1(struct parse_node *, node) {
  if (node) {
    c_generate_node(node);
  } else {
    ins_f_byte(F_CONST0);
  }
  ins_f_byte(F_RETURN);
}

void c_generate_break_point() {
  ins_f_byte(F_BREAK_POINT);
}

void c_generate_break() {
  if (current_break_address & BREAK_ON_STACK) {
    ins_f_byte(F_BREAK);
  } else {
    /* form a linked list of break addresses */
    ins_f_byte(F_JUMP);
    ins_short(current_break_address);
    current_break_address = CURRENT_PROGRAM_SIZE - 2;
  }
}

void c_generate_continue() {
  if (switches) {
    ins_f_byte(F_POP_BREAK);
    ins_byte(switches);
  }
  /* form a linked list of the continue addresses */
  ins_f_byte(F_JUMP);
  ins_short(current_continue_address);
  current_continue_address = CURRENT_PROGRAM_SIZE - 2;
}

void
c_generate_forward_jump() {
  ins_f_byte(F_JUMP);
  ins_short(current_forward_jump);
  current_forward_jump = CURRENT_PROGRAM_SIZE - 2;
}

void c_generate_forward_branch P1(char, b) {
  ins_f_byte(b);
  ins_short(current_forward_branch);
  current_forward_branch = CURRENT_PROGRAM_SIZE - 2;
}

void
c_update_forward_jump() {
  int i = read_short(current_forward_jump);
  upd_short(current_forward_jump, CURRENT_PROGRAM_SIZE);
  current_forward_jump = i;
}

void
c_update_forward_branch() {
  int i = read_short(current_forward_branch);
  upd_short(current_forward_branch, CURRENT_PROGRAM_SIZE - current_forward_branch);
  current_forward_branch = i;
}

void
c_generate_continues() {
  int next_addr;

  /* traverse the linked list filling in the current_continue_address
     required by each "continue" statement.
     */
  for (; current_continue_address > 0;
       current_continue_address = next_addr) {
    next_addr = read_short(current_continue_address);
    upd_short(current_continue_address, CURRENT_PROGRAM_SIZE);
  }
}

void
c_save_position() {
  push_address();
}

void
c_branch_backwards P1(char, b) {
    if (b) {
	ins_f_byte(b);
	ins_short(CURRENT_PROGRAM_SIZE - pop_address());
    } else
	pop_address();
}

void
c_update_breaks() {
  int next_addr;

  /* traverse the linked list filling in the current_break_address
     required by each "break" statement.
     */
  for (;current_break_address > 0; current_break_address = next_addr) {
    next_addr = read_short(current_break_address);
    upd_short(current_break_address, CURRENT_PROGRAM_SIZE);
  }
}

void
c_update_continues() {
  int next_addr;

  /* traverse the linked list filling in the current_continue_address
     required by each "continue" statement.
     */
  for (;current_continue_address > 0; current_continue_address = next_addr) {
    next_addr = read_short(current_continue_address);
    upd_short(current_continue_address, CURRENT_PROGRAM_SIZE);
  }
}

void
c_save_loop_info() {
  push_explicit(current_continue_address);
  push_explicit(current_break_address);
  /* keep track of # of nested switches prior to this while
     so that we'll know how many to pop from the break stack in
     the event of a "continue;" in the body of a switch().
     */
  DEBUG_CHECK(switch_sptr == SWITCH_STACK_SIZE,
	      "switch_stack overflow\n");
  switch_stack[switch_sptr++] = switches;
  switches = 0;

  /* each 'continue' statement will add to a linked list of slots that
     need filled in once the continue_address is known.  The delimeters
     serve as markers for the ends of the lists.
     */
  current_continue_address = CONTINUE_DELIMITER;
  current_break_address = BREAK_DELIMITER;
}

void
c_restore_loop_info() {
    DEBUG_CHECK(switch_sptr == 0,
		"switch_stack underflow\n");
  switches = switch_stack[--switch_sptr];
  
  current_break_address = pop_address();
  current_continue_address = pop_address();
}

void c_start_switch() {
  push_explicit(current_case_number_heap);
  push_explicit(current_case_string_heap);
  push_explicit(zero_case_label);
  push_explicit(current_break_address);
  ins_f_byte(F_SWITCH);
  ins_byte(0xff); /* kind of table */
  ins_short(0); /* address of table */
  current_break_address = mem_block[current_block].current_size |
    BREAK_ON_STACK | BREAK_FROM_CASE ;
  ins_short(0); /* break address to push, table is entered before */
  ins_short(0); /* default address */
}

void
c_generate_else() {
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
c_initialize_parser() {
    current_block = A_PROGRAM;
    init_dirty = -1;
    registers_in_use = 0; registers_need_free = 0; dirty_register = -1;
#ifdef DEBUG
    on_stack = 0; num_prefab = 0;
#endif
    needs_comma = 0;
    stack_sp = simul_stack;
    ins_string("#include \"lpc_to_c.h\"\n");
}
#endif
