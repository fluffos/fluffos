/*
 * Defines and types for the compiler parse trees.
 */

#ifndef _TREES_H
#define _TREES_H

#define NODES_PER_BLOCK         256

/* special node types */
#define NODE_CONDITIONAL        -1
#define NODE_ASSOC              -2
#define NODE_COMMA              -3
#define NODE_BREAK              -4
#define NODE_CONTINUE           -5
#define NODE_STATEMENTS         -6
#define NODE_WHILE              -7
#define NODE_DO_WHILE           -8
#define NODE_FOR                -9
#define NODE_SWITCH_NUMBERS     -10
#define NODE_SWITCH_DIRECT      -11
#define NODE_SWITCH_STRINGS     -12
#define NODE_CASE_NUMBER        -13
#define NODE_CASE_STRING        -14
#define NODE_DEFAULT            -15
#define NODE_IF                 -16

struct parse_node {
    short kind;
    short line;
    char type;
    union {
	int number;
	float real;
	struct parse_node *expr;
    } v;
    struct parse_node *left;
    struct parse_node *right;
};

struct parse_node_block {
    struct parse_node_block *next;
    struct parse_node nodes[NODES_PER_BLOCK];
};

#define CREATE_NODE(x,y) (x) = new_node(); (x)->kind = y;
#define NODE_NO_LINE(x,y) (x) = new_node_no_line(); (x)->kind = y;
#define CREATE_TYPED_NODE(x, y, z) (x)=new_node(); (x)->kind=y; (x)->type=z

/* tree functions */
void free_tree PROT((void));
void release_tree PROT((void));
void lock_expressions PROT((void));
void unlock_expressions PROT((void));
/* node functions */
struct parse_node *new_node PROT((void));
struct parse_node *new_node_no_line PROT((void));
struct parse_node *make_branched_node PROT((short, char, 
				struct parse_node *, struct parse_node *));
/* parser grammar functions */
struct parse_node *binary_int_op PROT((struct parse_node *, 
				       struct parse_node *, char, char *));
struct parse_node *insert_pop_value PROT((struct parse_node *));

#endif
