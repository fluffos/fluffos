/*
 * Support for parse trees for the compiler.
 *
 * Added by Beek (Tim Hollebeek) 9/29/94.  Only converting expression parsing
 *    to parse trees at this point; the rest of code generation will likely
 *    follow later.
 *
 */

#include "trees.h"
#include "compiler_shared.h"
#include "opcodes.h"

/* our globals */
static struct parse_node_block *parse_block_list = 0;
static struct parse_node_block *free_block_list = 0;

static struct parse_node *next_node = 0;
static struct parse_node *last_node = 0;

static struct parse_node *current_lock_node = 0;
static struct parse_node_block *current_lock_block = 0;

/* called by code generation when it is done with the tree */
void
free_tree() {
    struct parse_node_block *cur_block;

    if (cur_block = current_lock_block) {
      if (cur_block->next) {
	do {
	  cur_block = cur_block->next;
	} while (cur_block->next);
	cur_block->next = free_block_list;
	free_block_list = current_lock_block->next;
	current_lock_block->next = 0;
      }
      next_node = current_lock_node;
      last_node = &current_lock_block->nodes[NODES_PER_BLOCK];
      /* reserve current_lock_node again */
      new_node();
    } else {
      if (!(cur_block = parse_block_list)) return;

      while (cur_block->next) cur_block = cur_block->next;

      /* put all the blocks in the free list */
      cur_block->next = free_block_list;
      free_block_list = parse_block_list;
      parse_block_list = 0;
      next_node = 0;
      last_node = 0;
    }
}

void
lock_expressions() {
  /* save the previous lock info in a node so this is reentrant. */
  struct parse_node *save_node = new_node();
  
  save_node->right = current_lock_node;
  save_node->v.pnblock = current_lock_block;

  current_lock_node = save_node;
  current_lock_block = parse_block_list;
  while (current_lock_block->next)
    current_lock_block = current_lock_block->next;
}

void
unlock_expressions() {
  struct parse_node *save_node = current_lock_node;

  current_lock_block = save_node->v.pnblock;
  current_lock_node = save_node->right;
}

/* called when the parser cleans up */
void
release_tree() {
    struct parse_node_block *cur_block;
    struct parse_node_block *next_block;
    
    current_lock_node = 0;
    current_lock_block = 0;

    free_tree();
    next_block = free_block_list;
    while (cur_block = next_block) {
	next_block = cur_block->next;
	FREE(cur_block);
    }
    free_block_list = 0;
}

/* get a new node to add to the tree */
struct parse_node *
new_node() {
    struct parse_node_block *cur_block;

    /* fast case */
    if (next_node < last_node)
	return next_node++;
    
    /* no more nodes in the current block; do we have a free one? */
    if (cur_block = free_block_list) {
	free_block_list = cur_block->next;
    } else {
	cur_block = (struct parse_node_block *)DMALLOC(sizeof(struct parse_node_block), 0, "new_node");
    }
    /* add to block list */
    cur_block->next = parse_block_list;
    parse_block_list = cur_block;
    /* point the nodes correctly */
    next_node = &cur_block->nodes[1];
    last_node = &cur_block->nodes[NODES_PER_BLOCK];
    return &cur_block->nodes[0];
}

/* quick routine to make a generic node */
struct parse_node *
make_node P3(short, kind, char, flags, char, type) {
    struct parse_node *ret;

    ret = new_node();
    ret->kind = kind;
    ret->flags = flags;
    ret->type = type;
    return ret;
}

/* quick routine to make a generic branched node */
struct parse_node *
make_branched_node P4(short, kind, char, type, 
		      struct parse_node *, l, struct parse_node *, r) {
    struct parse_node *ret;

    ret = new_node();
    ret->kind = kind;
    ret->flags = 0;
    ret->type = type;
    ret->left = l;
    ret->right = r;
    return ret;
}

/* create an optimized typical binary integer operator */
struct parse_node *
binary_int_op P4(struct parse_node *, l, struct parse_node *, r,
		 char, op, char *, name) {
    if (exact_types && !TYPE(l->type, TYPE_NUMBER)) {
	char buf[256];
	strcpy(buf, "Bad left argument to '");
	strcat(buf, name);
	strcat(buf, "' : \"");
	strcat(buf, get_type_name(l->type));
	strcat(buf, "\"");
	yyerror(buf);
    }
    if (exact_types && !TYPE(r->type,TYPE_NUMBER)) {
	char buf[256];
	strcpy(buf, "Bad right argument to '");
	strcat(buf, name);
	strcat(buf, "' : \"");
	strcat(buf, get_type_name(r->type));
	strcat(buf, "\"");
	yyerror(buf);
    }
    if ((l->flags & r->flags & E_CONST)
	&& BASIC_TYPE(l->type, TYPE_NUMBER)
	&& BASIC_TYPE(r->type, TYPE_NUMBER)) {
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
    } else {
	struct parse_node *tmp;
	tmp = make_branched_node(op, TYPE_NUMBER, l, r);
	/* if the expression is commutative, move the constant to the right
	 * node.  This allows constant folding of cases like 1 + x - 1
	 */
	if (l->flags & E_CONST) {
	    switch (l->kind) {
	    case F_OR:
	    case F_XOR:
	    case F_AND:
		tmp->left = r;
		tmp->right = l;
	    }
	}
	return tmp;
    }
}

struct parse_node *insert_pop_value P1(struct parse_node *, expr) {
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
	expr = make_branched_node(F_POP_VALUE, 0, 0, expr);
    }
    return expr;
}

