/*
 * avltree.h
 *
 * Written by Paul Vixie
 */

#ifndef _AVLTREE_H_
#define _AVLTREE_H_

typedef struct tree_s {
    struct tree_s *tree_l, *tree_r;	/* left & right branches */
    char *tree_p;		/* data */
    short tree_b;		/* balance information */
}      tree;

void tree_init (tree **);
char *tree_srch (tree *, int (*) (), char *);
void tree_add (tree **, int (*) (), char *, int (*) ());
int tree_delete (tree **, int (*) (), char *, int (*) ());
int tree_trav (tree **, int (*) ());
void tree_mung (tree **, int (*) ());

#endif				/* _AVLTREE_H_ */
