/*
 * Defines and types for the compiler parse trees.
 */

#ifndef _TREES_H
#define _TREES_H
#include "lint.h"

#define NODES_PER_BLOCK         256

/* special node types */
#define NODE_CONDITIONAL        -1
#define NODE_ASSOC              -2
#define NODE_COMMA              -3

/* flags */
#define E_CONST                 1

struct parse_node {
    short kind;
    char flags;
    char type;
    union {
	int number;
	float real;
	struct parse_node *expr;
	struct parse_node_block *pnblock; /* used by lock_expressions() */
    } v;
    struct parse_node *left;
    struct parse_node *right;
};

struct parse_node_block {
    struct parse_node_block *next;
    struct parse_node nodes[NODES_PER_BLOCK];
};

/* tree functions */
void free_tree PROT((void));
void release_tree PROT((void));
void lock_expressions PROT((void));
void unlock_expressions PROT((void));
/* node functions */
struct parse_node *new_node PROT((void));
struct parse_node *make_node PROT((short, char, char));
struct parse_node *make_branched_node PROT((short, char, 
				struct parse_node *, struct parse_node *));
/* parser grammar functions */
struct parse_node *binary_int_op PROT((struct parse_node *, 
				       struct parse_node *, char, char *));
struct parse_node *insert_pop_value PROT((struct parse_node *));

#endif
