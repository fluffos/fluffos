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

void tree_init PROT((tree **));
char *tree_srch PROT((tree *, int (*) (), char *));
void tree_add PROT((tree **, int (*) (), char *, int (*) ()));
int tree_delete PROT((tree **, int (*) (), char *, int (*) ()));
int tree_trav PROT((tree **, int (*) ()));
void tree_mung PROT((tree **, int (*) ()));

#endif				/* _AVLTREE_H_ */
