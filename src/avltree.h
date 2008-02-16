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
char *tree_srch (tree *, int (*) (void *, void *), char *);
void tree_add (tree **, int (*) (void *, void *), char *, int (*) (void *));
int tree_delete (tree **, int (*) (void *, void *), char *, int (*) (void *));
int tree_trav (tree **, int (*) (void *));
void tree_mung (tree **, int (*) (void *));

#endif				/* _AVLTREE_H_ */
