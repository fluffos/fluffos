/*
 * avltree.c
 *
 * This program text was created by Paul Vixie using examples from the book:
 * "Algorithms & Data Structures," Niklaus Wirth, Prentice-Hall, 1986, ISBN
 * 0-13-022005-1.  This code and associated documentation is hereby placed
 * in the public domain.
 */

/********************************* README *********************************

AVL Trees V1.0
24-July-1987
Paul Vixie

This library and test program are useful for creating and using balanced
binary trees (AVL trees).  The tree is held in memory, using malloc(3) to
allocate storage.  A better version would allow file-based trees in
addition; once memory mapped files hit the UNIX(tm) community, this will
be much easier to do.  In the meanwhile, these routines have been very
useful to be for symbol tables and the like.  (Yes, I'm sure hashing is
better in some way, but I've used this for symbol tables, just the same.)

I cannot take credit for the algorithms.  See "Algorithms & Data Structures,"
Niklaus Wirth, Prentice-Hall 1986, ISBN 0-13-022005-1.  This is an update of
Wirth's previous book, titled "Algorythms + Data Structures = Programs,"
which used Pascal as the language for examples.  This later book uses the
newer Modula-2 for it's examples; this tree code was created using the
Modula-2 examples as guidelines.  At the time I typed this stuff in (about
a year ago, in July 1987), I understood how it all worked.  Today, well...

This code is hereby placed in the public domain, unless restrictions apply
from Prentice-Hall on the algorithms themselves.  If you use or redistribute
this code, please leave my name (and Wirth's) in the comments.

**************************************************************************/

#include "std.h"
#include "avltree.h"

/*
 * Prototypes for local functions
 */
static void sprout (tree **, char *, int *, int (*) (void *, void *), int (*) (void *));
static int avldelete (tree **, int (*) (void *, void *), char *, int (*) (void *), int *, int *);
static void del (tree **, int *, tree **, int (*) (void *), int *);
static void balanceL (tree **, int *);
static void balanceR (tree **, int *);


void tree_init (tree ** ppr_tree)
{
    *ppr_tree = NULL;
    return;
}


char *tree_srch(tree *ppr_tree, int (*pfi_compare) (void *, void *), char *pc_user){
    register int i_comp;

    while (ppr_tree) {
	i_comp = (*pfi_compare) (pc_user, ppr_tree->tree_p);
	if (i_comp > 0) {
	    ppr_tree = ppr_tree->tree_r;
	    continue;
	}
	if (i_comp < 0) {
	    ppr_tree = ppr_tree->tree_l;
	    continue;
	}
	/*
	 * not higher, not lower... this must be the one.
	 */
	return ppr_tree->tree_p;
    }

    /*
     * grounded. NOT found.
     */
    return NULL;
}


void tree_add(tree **ppr_tree, int (*pfi_compare) (void *, void *), char *pc_user, int (*pfi_delete) (void *)){
    int i_balance = 0;

    sprout(ppr_tree, pc_user, &i_balance, pfi_compare, pfi_delete);
    return;
}


static void sprout(tree **ppr, char *pc_data, int *pi_balance, int (*pfi_compare) (void *, void *), int (*pfi_delete) (void *)){
    tree *p1, *p2;
    int cmp;

    /*
     * are we grounded?  if so, add the node "here" and set the rebalance
     * flag, then exit.
     */
    if (!*ppr) {
	*ppr = ALLOCATE(tree, TAG_UID, "sprout");
	(*ppr)->tree_l = NULL;
	(*ppr)->tree_r = NULL;
	(*ppr)->tree_b = 0;
	(*ppr)->tree_p = pc_data;
	*pi_balance = 1;
	return;
    }
    /*
     * compare the data using routine passed by caller.
     */
    cmp = (*pfi_compare) (pc_data, (*ppr)->tree_p);

    /*
     * if LESS, prepare to move to the left.
     */
    if (cmp < 0) {
	sprout(&(*ppr)->tree_l, pc_data, pi_balance,
	       pfi_compare, pfi_delete);
	if (*pi_balance) {	/* left branch has grown longer */
	    switch ((*ppr)->tree_b) {
	    case 1:		/* right branch WAS longer; balance is ok now */
		(*ppr)->tree_b = 0;
		*pi_balance = 0;
		break;
	    case 0:		/* balance WAS okay; now left branch longer */
		(*ppr)->tree_b = -1;
		break;
	    case -1:
		/* left branch was already too long. rebalnce */
		p1 = (*ppr)->tree_l;
		if (p1->tree_b == -1) {	/* LL */
		    (*ppr)->tree_l = p1->tree_r;
		    p1->tree_r = *ppr;
		    (*ppr)->tree_b = 0;
		    *ppr = p1;
		} else {	/* double LR */
		    p2 = p1->tree_r;
		    p1->tree_r = p2->tree_l;
		    p2->tree_l = p1;

		    (*ppr)->tree_l = p2->tree_r;
		    p2->tree_r = *ppr;

		    if (p2->tree_b == -1)
			(*ppr)->tree_b = 1;
		    else
			(*ppr)->tree_b = 0;

		    if (p2->tree_b == 1)
			p1->tree_b = -1;
		    else
			p1->tree_b = 0;
		    *ppr = p2;
		}		/* else */
		(*ppr)->tree_b = 0;
		*pi_balance = 0;
	    }			/* switch */
	}			/* if */
	return;
    }				/* if */
    /*
     * if MORE, prepare to move to the right.
     */
    if (cmp > 0) {
	sprout(&(*ppr)->tree_r, pc_data, pi_balance,
	       pfi_compare, pfi_delete);
	if (*pi_balance) {	/* right branch has grown longer */
	    switch ((*ppr)->tree_b) {
	    case -1:
		(*ppr)->tree_b = 0;
		*pi_balance = 0;
		break;
	    case 0:
		(*ppr)->tree_b = 1;
		break;
	    case 1:
		p1 = (*ppr)->tree_r;
		if (p1->tree_b == 1) {	/* RR */
		    (*ppr)->tree_r = p1->tree_l;
		    p1->tree_l = *ppr;
		    (*ppr)->tree_b = 0;
		    *ppr = p1;
		} else {	/* double RL */
		    p2 = p1->tree_l;
		    p1->tree_l = p2->tree_r;
		    p2->tree_r = p1;

		    (*ppr)->tree_r = p2->tree_l;
		    p2->tree_l = *ppr;

		    if (p2->tree_b == 1)
			(*ppr)->tree_b = -1;
		    else
			(*ppr)->tree_b = 0;

		    if (p2->tree_b == -1)
			p1->tree_b = 1;
		    else
			p1->tree_b = 0;

		    *ppr = p2;
		}		/* else */
		(*ppr)->tree_b = 0;
		*pi_balance = 0;
	    }			/* switch */
	}			/* if */
	return;
    }				/* if */
    /*
     * not less, not more: this is the same key!  replace...
     */
    *pi_balance = 0;
    if (pfi_delete)
	(*pfi_delete) ((*ppr)->tree_p);
    (*ppr)->tree_p = pc_data;
    return;
}


int tree_delete(tree **ppr_p, int (*pfi_compare) (void *, void *), char *pc_user, int (*pfi_uar) (void *)){
    int i_balance = 0, i_uar_called = 0;

    return avldelete(ppr_p, pfi_compare, pc_user, pfi_uar,
		  &i_balance, &i_uar_called);
}


static int avldelete(tree **ppr_p, int (*pfi_compare) (void *, void *), char *pc_user, int (*pfi_uar) (void *), int *pi_balance, int *pi_uar_called){
    tree *pr_q;
    int i_comp, i_ret;

    if (*ppr_p == NULL) {
	return 0;
    }
    i_comp = (*pfi_compare) ((*ppr_p)->tree_p, pc_user);
    if (i_comp > 0) {
	i_ret = avldelete(&(*ppr_p)->tree_l, pfi_compare, pc_user, pfi_uar,
		       pi_balance, pi_uar_called);
	if (*pi_balance)
	    balanceL(ppr_p, pi_balance);
    } else if (i_comp < 0) {
	i_ret = avldelete(&(*ppr_p)->tree_r, pfi_compare, pc_user, pfi_uar,
		       pi_balance, pi_uar_called);
	if (*pi_balance)
	    balanceR(ppr_p, pi_balance);
    } else {
	pr_q = *ppr_p;
	if (pr_q->tree_r == NULL) {
	    *ppr_p = pr_q->tree_l;
	    *pi_balance = 1;
	} else if (pr_q->tree_l == NULL) {
	    *ppr_p = pr_q->tree_r;
	    *pi_balance = 1;
	} else {
	    del(&pr_q->tree_l, pi_balance, &pr_q, pfi_uar,
		pi_uar_called);
	    if (*pi_balance)
		balanceL(ppr_p, pi_balance);
	}
	FREE(pr_q);
	if (!*pi_uar_called && pfi_uar)
	    (*pfi_uar) (pr_q->tree_p);
	i_ret = 1;
    }
    return i_ret;
}


static void del(tree **ppr_r, int *pi_balance, tree **ppr_q, int (*pfi_uar) (void *), int *pi_uar_called){
    if ((*ppr_r)->tree_r != NULL) {
	del(&(*ppr_r)->tree_r, pi_balance, ppr_q, pfi_uar,
	    pi_uar_called);
	if (*pi_balance)
	    balanceR(ppr_r, pi_balance);
    } else {
	if (pfi_uar)
	    (*pfi_uar) ((*ppr_q)->tree_p);
	*pi_uar_called = 1;
	(*ppr_q)->tree_p = (*ppr_r)->tree_p;
	*ppr_q = *ppr_r;
	*ppr_r = (*ppr_r)->tree_l;
	*pi_balance = 1;
    }

    return;
}


static void balanceL (tree ** ppr_p, int * pi_balance){
    tree *p1, *p2;
    int b1, b2;

    switch ((*ppr_p)->tree_b) {
    case -1:
	(*ppr_p)->tree_b = 0;
	break;
    case 0:
	(*ppr_p)->tree_b = 1;
	*pi_balance = 0;
	break;
    case 1:
	p1 = (*ppr_p)->tree_r;
	b1 = p1->tree_b;
	if (b1 >= 0) {
	    (*ppr_p)->tree_r = p1->tree_l;
	    p1->tree_l = *ppr_p;
	    if (b1 == 0) {
		(*ppr_p)->tree_b = 1;
		p1->tree_b = -1;
		*pi_balance = 0;
	    } else {
		(*ppr_p)->tree_b = 0;
		p1->tree_b = 0;
	    }
	    *ppr_p = p1;
	} else {
	    p2 = p1->tree_l;
	    b2 = p2->tree_b;
	    p1->tree_l = p2->tree_r;
	    p2->tree_r = p1;
	    (*ppr_p)->tree_r = p2->tree_l;
	    p2->tree_l = *ppr_p;
	    if (b2 == 1)
		(*ppr_p)->tree_b = -1;
	    else
		(*ppr_p)->tree_b = 0;
	    if (b2 == -1)
		p1->tree_b = 1;
	    else
		p1->tree_b = 0;
	    *ppr_p = p2;
	    p2->tree_b = 0;
	}
    }
    return;
}


static void balanceR (tree ** ppr_p, int * pi_balance){
    tree *p1, *p2;
    int b1, b2;

    switch ((*ppr_p)->tree_b) {
    case 1:
	(*ppr_p)->tree_b = 0;
	break;
    case 0:
	(*ppr_p)->tree_b = -1;
	*pi_balance = 0;
	break;
    case -1:
	p1 = (*ppr_p)->tree_l;
	b1 = p1->tree_b;
	if (b1 <= 0) {
	    (*ppr_p)->tree_l = p1->tree_r;
	    p1->tree_r = *ppr_p;
	    if (b1 == 0) {
		(*ppr_p)->tree_b = -1;
		p1->tree_b = 1;
		*pi_balance = 0;
	    } else {
		(*ppr_p)->tree_b = 0;
		p1->tree_b = 0;
	    }
	    *ppr_p = p1;
	} else {
	    p2 = p1->tree_r;
	    b2 = p2->tree_b;
	    p1->tree_r = p2->tree_l;
	    p2->tree_l = p1;
	    (*ppr_p)->tree_l = p2->tree_r;
	    p2->tree_r = *ppr_p;
	    if (b2 == -1)
		(*ppr_p)->tree_b = 1;
	    else
		(*ppr_p)->tree_b = 0;
	    if (b2 == 1)
		p1->tree_b = -1;
	    else
		p1->tree_b = 0;
	    *ppr_p = p2;
	    p2->tree_b = 0;
	}
    }
    return;
}


int tree_trav(tree **ppr_tree, int (*pfi_uar) (void *)){
    if (!*ppr_tree)
	return 1;

    if (!tree_trav(&(**ppr_tree).tree_l, pfi_uar))
	return 0;
    if (!(*pfi_uar) ((**ppr_tree).tree_p))
	return 0;
    if (!tree_trav(&(**ppr_tree).tree_r, pfi_uar))
	return 0;
    return 1;
}


void tree_mung(tree **ppr_tree, int (*pfi_uar) (void *)){
    if (*ppr_tree) {
	tree_mung(&(**ppr_tree).tree_l, pfi_uar);
	tree_mung(&(**ppr_tree).tree_r, pfi_uar);
	if (pfi_uar)
	    (*pfi_uar) ((**ppr_tree).tree_p);
	FREE(*ppr_tree);
	*ppr_tree = NULL;
    }
    return;
}
