/*
 * Support for parse trees for the compiler.
 *
 * Added by Beek (Tim Hollebeek) 9/29/94.  Only converting expression parsing
 *    to parse trees at this point; the rest of code generation will likely
 *    follow later.
 *
 */

#define SUPPRESS_COMPILER_INLINES
#include "std.h"
#include "lpc_incl.h"
#include "trees.h"
#include "compiler.h"
#include "opcodes.h"
#include "lex.h"

/* our globals */
static parse_node_block_t *parse_block_list = 0;
static parse_node_block_t *free_block_list = 0;

static parse_node_t *next_node = 0;
static parse_node_t *last_node = 0;

static int last_prog_size = 1;

/* called by code generation when it is done with the tree */
void
free_tree() {
    parse_node_block_t *cur_block;

    if (!(cur_block = parse_block_list))
	return;

    while (cur_block->next) cur_block = cur_block->next;

    /* put all the blocks in the free list */
    cur_block->next = free_block_list;
    free_block_list = parse_block_list;
    parse_block_list = 0;
    next_node = 0;
    last_node = 0;
}

/* called when the parser cleans up */
void
release_tree() {
    parse_node_block_t *cur_block;
    parse_node_block_t *next_block;
    
    free_tree();
    next_block = free_block_list;
    while ((cur_block = next_block)) {
	next_block = cur_block->next;
	FREE(cur_block);
    }
    free_block_list = 0;
    last_prog_size = 1;
}

/* get a new node to add to the tree */
parse_node_t *
new_node() {
    parse_node_block_t *cur_block;

    /* fast case */
    if (next_node < last_node) {
	next_node->line = current_line_base + current_line;
	return next_node++;
    }

    /* no more nodes in the current block; do we have a free one? */
    if ((cur_block = free_block_list)) {
	free_block_list = cur_block->next;
    } else {
	cur_block = ALLOCATE(parse_node_block_t, TAG_COMPILER, "new_node");
    }
    /* add to block list */
    cur_block->next = parse_block_list;
    parse_block_list = cur_block;
    /* point the nodes correctly */
    next_node = &cur_block->nodes[0];
    last_node = &cur_block->nodes[NODES_PER_BLOCK];
    next_node->line = current_line_base + current_line;
    return next_node++;
}

/* get a new node to add to the tree, but don't count it for line # purposes
 * This should be used for nodes that hold expressions together but don't
 * generate any code themselves (NODE_IF, etc)
 */
parse_node_t *
new_node_no_line() {
    parse_node_block_t *cur_block;

    /* fast case */
    if (next_node < last_node) {
      next_node->line = 0;
      return next_node++;
    }    
    /* no more nodes in the current block; do we have a free one? */
    if ((cur_block = free_block_list)) {
      free_block_list = cur_block->next;
    } else {
	cur_block = ALLOCATE(parse_node_block_t, TAG_COMPILER, "new_node");
    }
    /* add to block list */
    cur_block->next = parse_block_list;
    parse_block_list = cur_block;
    /* point the nodes correctly */
    next_node = &cur_block->nodes[0];
    last_node = &cur_block->nodes[NODES_PER_BLOCK];
    next_node->line = 0;
    return next_node++;
}

/* quick routine to make a generic branched node */
parse_node_t *
make_branched_node P4(short, kind, char, type, 
		      parse_node_t *, l, parse_node_t *, r) {
    parse_node_t *ret;

    ret = new_node();
    ret->kind = kind;
    ret->type = type;
    ret->l.expr = l;
    ret->r.expr = r;
    return ret;
}

/* create an optimized typical binary integer operator */
parse_node_t *
binary_int_op P4(parse_node_t *, l, parse_node_t *, r,
		 char, op, char *, name) {
    parse_node_t *ret;
    
    if (exact_types){
	if (!IS_TYPE(l->type, TYPE_NUMBER)) {
	    char buf[256];
	    strcpy(buf, "Bad left argument to '");
	    strcat(buf, name);
	    strcat(buf, "' : \"");
	    strcat(buf, get_type_name(l->type));
	    strcat(buf, "\"");
	    yyerror(buf);
	}
	if (!IS_TYPE(r->type,TYPE_NUMBER)) {
	    char buf[256];
	    strcpy(buf, "Bad right argument to '");
	    strcat(buf, name);
	    strcat(buf, "' : \"");
	    strcat(buf, get_type_name(r->type));
	    strcat(buf, "\"");
	    yyerror(buf);
	}
    }
    if (l->kind == NODE_NUMBER) {
      if (r->kind == NODE_NUMBER) {
          switch (op) {
          case F_OR: l->v.number |= r->v.number; break;
          case F_XOR: l->v.number ^= r->v.number; break;
          case F_AND: l->v.number &= r->v.number; break;
          case F_LSH: l->v.number <<= r->v.number; break;
          case F_RSH: l->v.number >>= r->v.number; break;
          case F_MOD:
              if (r->v.number == 0) {
                  yyerror("Modulo by zero constant");
                  break;
              }
             l->v.number %= r->v.number; break;
          default: fatal("Unknown opcode in binary_int_op()\n");
          }
          return l;
      }
      switch (op) {
      case F_OR:
      case F_XOR:
      case F_AND:
	  CREATE_BINARY_OP(ret, op, TYPE_NUMBER, r, l);
	  return ret;
      }
    }
    CREATE_BINARY_OP(ret, op, TYPE_NUMBER, l, r);
    return ret;
}

parse_node_t *make_range_node P4(int, code, parse_node_t *, expr,
                                      parse_node_t *, l,
                                      parse_node_t *, r) {
    parse_node_t *newnode;

    if (r) {
	CREATE_TERNARY_OP(newnode, code, 0, l, r, expr);
    } else {
	CREATE_BINARY_OP(newnode, code, 0, l, expr);
    }
    
    if (exact_types){
        switch(expr->type){
            case TYPE_ANY:
            case TYPE_STRING:
            case TYPE_BUFFER:
                newnode->type = expr->type;
                break;

            default:
                if (expr->type & TYPE_MOD_ARRAY) newnode->type = expr->type;
                else{
                    type_error("Bad type of argument used for range: ", expr->type);
                    newnode->type = TYPE_ANY;
                }
        }

        if (!IS_TYPE(l->type, TYPE_NUMBER))
            type_error("Bad type of left index to range operator", l->type);
        if (r && !IS_TYPE(r->type, TYPE_NUMBER))
            type_error("Bad type of right index to range operator", r->type);
    } else newnode->type = TYPE_ANY;
    return newnode;
}

parse_node_t *insert_pop_value P1(parse_node_t *, expr) {
    parse_node_t *replacement;

    if (expr->type == TYPE_NOVALUE) {
	expr->type = TYPE_VOID;
	return expr;
    }
    switch (expr->kind) {
    case NODE_BINARY_OP:
	switch (expr->v.number) {
	case F_EQ: case F_NE: case F_GT: case F_GE: case F_LT: case F_LE:
	    yywarn("Value of conditional expression is unused");
	    break;
	case F_ASSIGN:
	    if (IS_NODE(expr->r.expr, NODE_OPCODE_1, F_LOCAL_LVALUE)) {
		int tmp = expr->r.expr->l.number;
		expr->kind = NODE_UNARY_OP_1;
		expr->r.expr = expr->l.expr;
		expr->v.number = F_VOID_ASSIGN_LOCAL;
		expr->l.number = tmp;
	    } else expr->v.number = F_VOID_ASSIGN;
	    return expr;
	case F_ADD_EQ: 
	    expr->v.number = F_VOID_ADD_EQ;
	    return expr;
	}
	break;
    case NODE_UNARY_OP:
	switch (expr->v.number) {
	case F_PRE_INC: case F_POST_INC:
	    expr->v.number = F_INC;
	    return expr;
	case F_PRE_DEC: case F_POST_DEC: 
	    expr->v.number = F_DEC;
	    return expr;
	}
	break;
    case NODE_NUMBER:
    case NODE_STRING:
    case NODE_REAL: 
	return 0;
    }
    CREATE_UNARY_OP(replacement, F_POP_VALUE, 0, expr);
    return replacement;
}

