/*
 * Support for parse trees for the compiler.
 *
 * Added by Beek (Tim Hollebeek) 9/29/94.  Only converting expression parsing
 *    to parse trees at this point; the rest of code generation will likely
 *    follow later.
 *
 */

#include "std.h"
#include "trees.h"
#include "compiler.h"
#include "opcodes.h"
#include "lex.h"
#include "simulate.h"

/* our globals */
static struct parse_node_block *parse_block_list = 0;
static struct parse_node_block *free_block_list = 0;

static struct parse_node *next_node = 0;
static struct parse_node *last_node = 0;

static int last_prog_size = 1;

/* called by code generation when it is done with the tree */
void
free_tree() {
    struct parse_node_block *cur_block;

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
    struct parse_node_block *cur_block;
    struct parse_node_block *next_block;
    
    free_tree();
    next_block = free_block_list;
    while (cur_block = next_block) {
	next_block = cur_block->next;
	FREE(cur_block);
    }
    free_block_list = 0;
    last_prog_size = 1;
}

/* get a new node to add to the tree */
struct parse_node *
new_node() {
    struct parse_node_block *cur_block;

    /* fast case */
    if (next_node < last_node) {
	next_node->line = current_line_base + current_line;
	return next_node++;
    }

    /* no more nodes in the current block; do we have a free one? */
    if (cur_block = free_block_list) {
	free_block_list = cur_block->next;
    } else {
	cur_block = ALLOCATE(struct parse_node_block, TAG_COMPILER, "new_node");
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
struct parse_node *
new_node_no_line() {
    struct parse_node_block *cur_block;

    /* fast case */
    if (next_node < last_node) {
      next_node->line = 0;
      return next_node++;
    }    
    /* no more nodes in the current block; do we have a free one? */
    if (cur_block = free_block_list) {
      free_block_list = cur_block->next;
    } else {
	cur_block = ALLOCATE(struct parse_node_block, TAG_COMPILER, "new_node");
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
struct parse_node *
make_branched_node P4(short, kind, char, type, 
		      struct parse_node *, l, struct parse_node *, r) {
    struct parse_node *ret;

    ret = new_node();
    ret->kind = kind;
    ret->type = type;
    ret->l.expr = l;
    ret->r.expr = r;
    return ret;
}

/* create an optimized typical binary integer operator */
struct parse_node *
binary_int_op P4(struct parse_node *, l, struct parse_node *, r,
		 char, op, char *, name) {
    if (exact_types){
	if (!BASIC_TYPE(l->type, TYPE_NUMBER)){
	    char buf[256];
	    strcpy(buf, "Bad left argument to '");
	    strcat(buf, name);
	    strcat(buf, "' : \"");
	    strcat(buf, get_type_name(l->type));
	    strcat(buf, "\"");
	    yyerror(buf);
	}
	if (!BASIC_TYPE(r->type,TYPE_NUMBER)) {
	    char buf[256];
	    strcpy(buf, "Bad right argument to '");
	    strcat(buf, name);
	    strcat(buf, "' : \"");
	    strcat(buf, get_type_name(r->type));
	    strcat(buf, "\"");
	    yyerror(buf);
	}
    }
    if (l->kind == F_NUMBER) {
      if (r->kind == F_NUMBER) {
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
          return make_branched_node(op, TYPE_NUMBER, r, l);
      }
    }
    return make_branched_node(op, TYPE_NUMBER, l, r);
}

struct parse_node *make_range_node P4(int, code, struct parse_node *, expr,
                                      struct parse_node *, l,
                                      struct parse_node *, r) {
    struct parse_node *newnode;

    newnode = make_branched_node(code, 0, l, r);
    newnode->v.expr = expr;

    if (exact_types){
        switch(expr->type){
            case TYPE_ANY:
            case TYPE_STRING:
            case TYPE_BUFFER:
                newnode->type = expr->type;
                break;

            default:
                if (expr->type & TYPE_MOD_POINTER) newnode->type = expr->type;
                else{
                    type_error("Bad type of argument used for range: ", expr->type);
                    newnode->type = TYPE_ANY;
                }
        }

        if (!BASIC_TYPE(l->type, TYPE_NUMBER))
            type_error("Bad type of left index to range operator", l->type);
        if (r && !BASIC_TYPE(r->type, TYPE_NUMBER))
            type_error("Bad type of right index to range operator", r->type);
    } else newnode->type = TYPE_ANY;
    return newnode;
}

struct parse_node *insert_pop_value P1(struct parse_node *, expr) {
    struct parse_node *replacement;

    switch (expr->kind) {
    case F_ASSIGN: expr->kind = F_VOID_ASSIGN; break;
    case F_ADD_EQ: expr->kind = F_VOID_ADD_EQ; break;
    case F_PRE_INC:
    case F_POST_INC: expr->kind = F_INC; break;
    case F_PRE_DEC:
    case F_POST_DEC: expr->kind = F_DEC; break;
    case F_NUMBER:
    case F_STRING:
    case F_REAL: expr = 0; break;
    case F_EQ:
    case F_NE:
    case F_GT:
    case F_GE:
    case F_LT:
    case F_LE:
	yywarn("Value of conditional expression is unused");
    default:
	NODE_NO_LINE(replacement, F_POP_VALUE);
	replacement->r.expr = expr;
	return replacement;
    }
    return expr;
}

