/*
 * Support for parse trees for the compiler.
 *
 * Added by Beek (Tim Hollebeek) 9/29/94.  Only converting expression parsing
 *    to parse trees at this point; the rest of code generation will likely
 *    follow later.
 *
 * Note: it did.  See ChangeLogs.
 *
 */

#define SUPPRESS_COMPILER_INLINES
#include "std.h"
#include "lpc_incl.h"
#include "compiler.h"
#include "opcodes.h"

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
make_branched_node (short kind, char type, 
                      parse_node_t * l, parse_node_t * r) {
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
binary_int_op (parse_node_t * l, parse_node_t * r,
                 char op, const char * name) {
    parse_node_t *ret;
    
    if (exact_types) {
        if (!IS_TYPE(l->type, TYPE_NUMBER)) {
            char buf[256];
            char *end = EndOf(buf);
            char *p;
            
            p = strput(buf, end, "Bad left argument to '");
            p = strput(p, end, name);
            p = strput(p, end, "' : \"");
            p = get_type_name(p, end, l->type);
            p = strput(p, end, "\"");
            yyerror(buf);
        }
        if (!IS_TYPE(r->type,TYPE_NUMBER)) {
            char buf[256];
            char *end = EndOf(buf);
            char *p;
            
            p = strput(buf, end, "Bad right argument to '");
            p = strput(p, end, name);
            p = strput(p, end, "' : \"");
            p = get_type_name(p, end, r->type);
            p = strput(p, end, "\"");
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

parse_node_t *make_range_node (int code, parse_node_t * expr,
                                      parse_node_t * l,
                                      parse_node_t * r) {
    parse_node_t *newnode;

    if (r) {
        CREATE_TERNARY_OP(newnode, code, 0, l, r, expr);
    } else {
        CREATE_BINARY_OP(newnode, code, 0, l, expr);
    }
    
    if (exact_types) {
        switch(expr->type) {
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

parse_node_t *insert_pop_value (parse_node_t * expr) {
    parse_node_t *replacement;

    if (!expr)
        return 0;
    if (expr->type == TYPE_NOVALUE) {
        expr->type = TYPE_VOID;
        return expr;
    }
    switch (expr->kind) {
    case NODE_EFUN:
        if (expr->v.number & NOVALUE_USED_FLAG) {
            expr->v.number &= ~NOVALUE_USED_FLAG;
            return expr;
        }
        break;
    case NODE_TWO_VALUES:
        /* (two-values expr1 expr2) where expr1 is already popped.
         * 
         * instead of: (pop (two-values expr1 expr2))
         * generated:  (two-values expr (pop expr2))
         *
         * both of which generate the same code, but the second optimizes
         * better in cases like: i++, j++
         *
         * we get: (two-values (inc i) (post-inc j))
         * first: (pop (two-values (inc i) (post-inc j)))
         * -> INC i; POST_INC j; POP
         * second: (two-values (inc i) (inc j))
         * -> INC i; INC j
         */
        if ((expr->r.expr = insert_pop_value(expr->r.expr)))
            return expr;
        return expr->l.expr;
    case NODE_IF:
        /* a NODE_IF that gets popped is a (x ? y : z);
         * propagate the pop in order to produce the same code as
         * if (x) y; else z;
         */
        expr->l.expr = insert_pop_value(expr->l.expr);
        expr->r.expr = insert_pop_value(expr->r.expr);

        if (!expr->l.expr && !expr->r.expr) {
            /* if both branches do nothing, don't bother with the test ... */
            return insert_pop_value(expr->v.expr);
        }
        return expr;
    case NODE_TERNARY_OP:
        switch (expr->r.expr->v.number) {
        case F_NN_RANGE: case F_RN_RANGE: case F_RR_RANGE: case F_NR_RANGE:
            expr->kind = NODE_TWO_VALUES;
            expr->l.expr = insert_pop_value(expr->l.expr);
            expr->r.expr->kind = NODE_TWO_VALUES;
            expr->r.expr->l.expr = insert_pop_value(expr->r.expr->l.expr);
            expr->r.expr->r.expr = insert_pop_value(expr->r.expr->r.expr);

            if (!expr->l.expr) {
                expr = expr->r.expr;
                if (!expr->l.expr)
                    return expr->r.expr;
                if (!expr->r.expr)
                    return expr->l.expr;
            } else {
                if (!expr->r.expr->l.expr) {
                    expr->r.expr = expr->r.expr->r.expr;
                    if (!expr->r.expr)
                        return expr->l.expr;
                } else {
                    if (!expr->r.expr->r.expr)
                        expr->r.expr = expr->r.expr->l.expr;
                }
            }
            return expr;
        }
        break;
    /* take advantage of the fact that opcodes don't clash */
    case NODE_CALL:
    case NODE_BINARY_OP:
    case NODE_UNARY_OP_1:
    case NODE_UNARY_OP:
    case NODE_OPCODE_1:
        switch (expr->v.number) {
        case F_AGGREGATE_ASSOC:
            /* This has to be done specially b/c of the way mapping constants
               are stored */
            return throw_away_mapping(expr);
        case F_AGGREGATE:
            return throw_away_call(expr);
        case F_PRE_INC: case F_POST_INC:
            expr->v.number = F_INC;
            return expr;
        case F_PRE_DEC: case F_POST_DEC: 
            expr->v.number = F_DEC;
            return expr;
        case F_NOT: case F_COMPL: case F_NEGATE:
            expr = insert_pop_value(expr->r.expr);
            return expr;
        case F_MEMBER:
            expr = insert_pop_value(expr->r.expr);
            return expr;
        case F_LOCAL: case F_GLOBAL: case F_REF:
            return 0;
        case F_EQ: case F_NE: case F_GT: case F_GE: case F_LT: case F_LE:
        case F_OR: case F_XOR: case F_AND: case F_LSH: case F_RSH:
        case F_ADD: case F_SUBTRACT: case F_MULTIPLY: case F_DIVIDE:
        case F_MOD: case F_RE_RANGE: case F_NE_RANGE: case F_RINDEX:
        case F_INDEX:
            if ((expr->l.expr = insert_pop_value(expr->l.expr))) {
                if ((expr->r.expr = insert_pop_value(expr->r.expr))) {
                    expr->kind = NODE_TWO_VALUES;
                    return expr;
                } else
                    return expr->l.expr;
            } else 
                return insert_pop_value(expr->r.expr);
            break;
        case F_ASSIGN:
            if (IS_NODE(expr->r.expr, NODE_OPCODE_1, F_LOCAL_LVALUE)) {
                long tmp = expr->r.expr->l.number;
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
    case NODE_PARAMETER:
    case NODE_ANON_FUNC: /* some dweeb threw away one? */
    case NODE_FUNCTION_CONSTRUCTOR:
        return 0;
    case NODE_NUMBER:
    case NODE_STRING:
    case NODE_REAL: 
        return 0;
    }
    CREATE_UNARY_OP(replacement, F_POP_VALUE, 0, expr);
    return replacement;
}

parse_node_t *pop_value (parse_node_t * pn) {
    if (pn) {
        parse_node_t *ret = insert_pop_value(pn);

        if (!ret) {
            if (pn->kind == NODE_BINARY_OP && pn->v.number >= F_EQ &&
                pn->v.number <= F_GT)
                yywarn("Value of conditional expression is unused");
            else
                yywarn("Expression has no side effects, and the value is unused");
        }
        return ret;
    }
    
    return 0;
}

int is_boolean (parse_node_t * pn) {
    switch (pn->kind) {
    case NODE_UNARY_OP:
        if (pn->v.number == F_NOT)
            return 1;
        return 0;
    case NODE_BINARY_OP:
        if (pn->v.number >= F_EQ && pn->v.number <= F_GT)
            return 1;
        return 0;
    case NODE_LAND_LOR:
    case NODE_BRANCH_LINK:
        return 1;
    }
    return 0;
}

parse_node_t *optimize_loop_test (parse_node_t * pn) {
    parse_node_t *ret;

    if (!pn) return 0;
    
    if (IS_NODE(pn, NODE_BINARY_OP, F_LT) &&
        IS_NODE(pn->l.expr, NODE_OPCODE_1, F_LOCAL)) {
        if (IS_NODE(pn->r.expr, NODE_OPCODE_1, F_LOCAL)) {
            CREATE_OPCODE_2(ret, F_LOOP_COND_LOCAL, 0,
                            pn->l.expr->l.number,
                            pn->r.expr->l.number);
        } else if (pn->r.expr->kind == NODE_NUMBER) {
            CREATE_OPCODE_2(ret, F_LOOP_COND_NUMBER, 0,
                            pn->l.expr->l.number,
                            pn->r.expr->v.number);
        } else
            ret = pn;
    } else if (IS_NODE(pn, NODE_UNARY_OP, F_POST_DEC) &&
               IS_NODE(pn->r.expr, NODE_OPCODE_1, F_LOCAL_LVALUE)) {
        long lvar = pn->r.expr->l.number;
        CREATE_OPCODE_1(ret, F_WHILE_DEC, 0, lvar);
    } else
        ret = pn;
    
    return ret;
}








