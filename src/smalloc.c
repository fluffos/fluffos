/* Satoria's malloc intended to be optimized for lpmud.
** this memory manager distinguishes between two sizes
** of blocks: small and large.  It manages them separately
** in the hopes of avoiding fragmentation between them.
** It expects small blocks to mostly be temporaries.
** It expects an equal number of future requests as small
** block deallocations.
**
** support for atari st/tt and FAST_FIT by amylaar @cs.tu-berlin.de
**
** adapted by Blackthorn@Genocide to work with MudOS 0.9.15 - 93/01/26
**
** Amiga Lattice support added by Robocoder@TMI-2
*/

#define IN_MALLOC_WRAPPER
#define NO_OPCODES
#include "std.h"
#include "file_incl.h"
#include "lpc_incl.h"
#include "simulate.h"
#include "comm.h"

#if defined(sparc)
#define MALLOC_ALIGN 8
#else
#define MALLOC_ALIGN 4
#endif

#define POINTER void *
#define FREE_RETURN_TYPE void
#define FREE_RETURN return;
#define SFREE_RETURN_TYPE FREE_RETURN_TYPE
#define SFREE_RETURN FREE_RETURN

#define FIT_STYLE_FAST_FIT

#undef LARGE_TRACE

#define fake(s)

#define SMALL_BLOCK_MAX_BYTES	32
#define SMALL_CHUNK_SIZE	0x4000
#define CHUNK_SIZE		0x40000

#define SMALL_BLOCK_MAX (SMALL_BLOCK_MAX_BYTES/SIZEOF_INT)

#define PREV_BLOCK	0x80000000
#define THIS_BLOCK	0x40000000
#define NO_REF  	0x20000000	/* check this in gcollect.c */
#define MASK		0x0FFFFFFF

#define MAGIC       0x17952932

/* SMALL BLOCK info */

typedef unsigned int u;

static u *last_small_chunk = 0;
static u *sfltable[SMALL_BLOCK_MAX] =
{0, 0, 0, 0, 0, 0, 0, 0};	/* freed list */
static u *next_unused = 0;
static u unused_size = 0;	/* until we need a new chunk */

/* LARGE BLOCK info */

#ifndef FIT_STYLE_FAST_FIT
static u *free_list = 0;
#endif				/* FIT_STYLE_FAST_FIT */
static u *start_next_block = 0;

/* STATISTICS */

static int small_count[SMALL_BLOCK_MAX] =
{0, 0, 0, 0, 0, 0, 0, 0};
static int small_total[SMALL_BLOCK_MAX] =
{0, 0, 0, 0, 0, 0, 0, 0};
static int small_max[SMALL_BLOCK_MAX] =
{0, 0, 0, 0, 0, 0, 0, 0};
static int small_free[SMALL_BLOCK_MAX] =
{0, 0, 0, 0, 0, 0, 0, 0};

typedef struct {
    unsigned counter, size;
}      t_stat;

#define count(a,b) { a.size+=(b); if ((b)<0) --a.counter; else ++a.counter; }
#define count_up(a,b)   { a.size+=(b); ++a.counter; }
#define count_back(a,b) { a.size-=(b); --a.counter; }

#if 0
int debugmalloc = 0;		/* Only used when debuging malloc() */
#endif

/********************************************************/
/*  SMALL BLOCK HANDLER					*/
/********************************************************/

static char *large_malloc (u, int);
static void large_free (char *);
static int malloc_size_mask (void);
static int malloced_size (POINTER);
static void show_block (u *);
static void remove_from_free_list (u *);
static void add_to_free_list (u *);
static void build_block (u *, u);
static void mark_block (u *);
static char *esbrk (u);
static int resort_free_list (void);
#ifdef DEBUG
static void walk_new_small_malloced (void (* ()));
#endif

#define s_size_ptr(p)	(p)
#define s_next_ptr(p)	((u **) (p+1))

t_stat small_alloc_stat =
{0, 0};
t_stat small_free_stat =
{0, 0};
t_stat small_chunk_stat =
{0, 0};

POINTER CDECL smalloc_malloc (size_t size)
{
    /* int i; */
    u *temp;

    DEBUG_CHECK(size == 0, "Malloc size 0.\n");
    if (size > SMALL_BLOCK_MAX_BYTES)
	return large_malloc(size, 0);

#if SIZEOF_PTR > SIZEOF_INT
    if (size < SIZEOF_PTR)
	size = SIZEOF_PTR;
#endif

    size = (size + 7) & ~3;	/* block size in bytes */
#define SIZE_INDEX(u_array, size) 	(*(u*) ((char*)u_array-8+size))
#define SIZE_PNT_INDEX(u_array, size)	(*(u**)((char*)u_array-8+size))
    /* i = (size - 8) >> 2; */
    count_up(small_alloc_stat, size);

    SIZE_INDEX(small_count, size) += 1;	/* update statistics */
    SIZE_INDEX(small_total, size) += 1;
    if (SIZE_INDEX(small_count, size) > SIZE_INDEX(small_max, size))
	SIZE_INDEX(small_max, size) = SIZE_INDEX(small_count, size);

    if (temp = SIZE_PNT_INDEX(sfltable, size)) {	/* allocate from the
							 * free list */
	count_back(small_free_stat, size);
	temp++;
	SIZE_PNT_INDEX(sfltable, size) = *(u **) temp;
	fake("From free list.");
	return (char *) temp;
    }				/* else allocate from the chunk */
    if (unused_size < size) {	/* no room in chunk, get another */
	/*
	 * don't waste this smaller block
	 */
	if (unused_size) {
	    count_up(small_free_stat, unused_size);
	    *s_size_ptr(next_unused) = unused_size >> 2;
	    *s_next_ptr(next_unused) = SIZE_PNT_INDEX(sfltable, unused_size);
	    SIZE_PNT_INDEX(sfltable, unused_size) = next_unused;
	}

	fake("Allocating new small chunk.");
	next_unused = (u *) large_malloc(SMALL_CHUNK_SIZE + SIZEOF_PTR, 1);
	if (next_unused == 0)
	    return 0;
	
	*next_unused = (u) last_small_chunk;
	last_small_chunk = next_unused++;
	count_up(small_chunk_stat, SMALL_CHUNK_SIZE +  SIZEOF_PTR);
	count_up(small_alloc_stat, SIZEOF_PTR);
	unused_size = SMALL_CHUNK_SIZE;
    } else
	fake("Allocated from chunk.");

    temp = (u *) s_next_ptr(next_unused);
    *s_size_ptr(next_unused) = size >> 2;
    unused_size -= size;
    if (unused_size < (SIZEOF_INT + SIZEOF_PTR)) {
	count_up(small_alloc_stat, unused_size);
	if ((size + unused_size) < (SMALL_BLOCK_MAX_BYTES + SIZEOF_INT)) {
	    /*
	     * try to avoid waste
	     */
	    size += unused_size;
	    *s_size_ptr(next_unused) = size >> 2;
	}
	unused_size = 0;
    }
    next_unused += size >> 2;

    fake("allocation from chunk successful\n");
    return (char *) temp;
}

#ifdef DEBUG
char *debug_free_ptr;
#endif				/* DEBUG */

static int malloc_size_mask()
{
    return MASK;
}

static int malloced_size (POINTER ptr)
{
    return (((u *) ptr)[-1] & MASK);
}

SFREE_RETURN_TYPE CDECL smalloc_free (POINTER ptr)
{
    u *block;
    u i;

    if (!ptr)
	SFREE_RETURN;

#ifdef DEBUG
    debug_free_ptr = ptr;
#endif				/* DEBUG */
    block = (u *) ptr;
    block -= 1;
    i = (*s_size_ptr(block) & MASK);
    if ((*s_size_ptr(block) & MASK) > SMALL_BLOCK_MAX + 1) {
	fake("sfree calls large_free");
	large_free(ptr);
	SFREE_RETURN
    }
    count_back(small_alloc_stat, i << 2);
    count_up(small_free_stat, i << 2);
    i -= 2;
    *s_next_ptr(block) = sfltable[i];
    sfltable[i] = block;
    small_free[i] += 1;
    fake("Freed");
    SFREE_RETURN
}

/************************************************/
/*	LARGE BLOCK HANDLER			*/
/************************************************/

#define BEST_FIT	0
#define FIRST_FIT	1
#define HYBRID		2

#define fit_style BEST_FIT
/* if this is a constant, evaluate at compile-time.... */
#ifndef fit_style
int fit_style = BEST_FIT;
#endif

#define l_size_ptr(p)		(p)
#define l_next_ptr(p)		(*((u **) (p+1)))
#define l_prev_ptr(p)		(*((u **) ((u **)(p+1)+1)))
#define l_next_block(p)		(p + (MASK & (*(p))) )
#define l_prev_block(p) 	(p - (MASK & (*(p-1))) )
#define l_prev_free(p)		(!(*p & PREV_BLOCK))
#define l_next_free(p)		(!(*l_next_block(p) & THIS_BLOCK))

static void show_block (u * ptr)
{
    printf("[%c%d: %d]  ", (*ptr & THIS_BLOCK ? '+' : '-'),
	   ptr, (*ptr & MASK));
}

#ifdef FIT_STYLE_FAST_FIT

#if defined(atarist) || defined (sun) || defined(AMIGA)
/* there is a type signed char */
typedef SIGNED char balance_t;

#define BALANCE_T_BITS 8
#else
typedef short balance_t;

#define BALANCE_T_BITS 16
#endif
#if (defined(atarist) && !defined(ATARI_TT)) || defined(sparc) || defined(AMIGA)
 /* try to avoid multiple shifts, because these are costly */
#define NO_BARREL_SHIFT
#endif

typedef struct free_block_s {
    u size;
    struct free_block_s *parent, *left, *right;
    balance_t balance;
    short align_dummy;
} free_block_t;

/* prepare two nodes for the free tree that will never be removed,
   so that we can always assume that the tree is and remains non-empty. */
/* some compilers don't understand forward declarations of static vars. */
extern free_block_t dummy2;

static free_block_t dummy = { 
    /* size */ 0, 
    /* parent */ &dummy2,
    /* left */ 0,
    /* right */ 0,
    /* balance */ 0
};

free_block_t dummy2 =
{
    /* size */ 0,
    /* parent */ 0, 
    /* left */ &dummy, 
    /* right */ 0,
    /* balance */ -1
};

static free_block_t *free_tree = &dummy2;

#ifdef DEBUG_AVL
static int inconsistency = 0;

static int check_avl (free_block_t * parent, free_block_t * p)
{
    int left, right;

    if (!p)
	return 0;
    left = check_avl(p, p->left);
    right = check_avl(p, p->right);
    if (p->balance != right - left || p->balance < -1 || p->balance > 1) {
	printf("Inconsistency in avl node!\n");
	printf("node:%x\n", p);
	printf("size: %d\n", p->size);
	printf("left node:%x\n", p->left);
	printf("left  height: %d\n", left);
	printf("right node:%x\n", p->right);
	printf("right height: %d\n", right);
	printf("alleged balance: %d\n", p->balance);
	inconsistency = 1;
    }
    if (p->parent != parent) {
	printf("Inconsistency in avl node!\n");
	printf("node:%x\n", p);
	printf("size: %d\n", p->size);
	printf("parent: %x\n", parent);
	printf("parent size: %d\n", parent->size);
	printf("alleged parent: %x\n", p->parent);
	printf("alleged parent size: %d\n", p->parent->size);
	printf("left  height: %d\n", left);
	printf("right height: %d\n", right);
	printf("alleged balance: %d\n", p->balance);
	inconsistency = 1;
    }
    return left > right ? left + 1 : right + 1;
}

/* this function returns a value so that it can be used in ,-expressions. */
static int do_check_avl()
{
    check_avl(0, free_tree);
    if (inconsistency) {
	fflush(stderr);
	fflush(stdout);
	fatal("Inconsistency could crash the driver\n");
    }
    return 0;
}
#endif				/* DEBUG_AVL */

t_stat large_free_stat;
static void remove_from_free_list (u * ptr)
{
    free_block_t *p, *q, *r, *s, *t;

    fake((do_check_avl(), "remove_from_free_list called"));
    p = (free_block_t *) (ptr + 1);
    count_back(large_free_stat, p->size << 2);
#ifdef DEBUG_AVL
    printf("node:%x\n", p);
    printf("size:%d\n", p->size);
#endif
    if (p->left) {
	if (q = p->right) {
	    fake("two childs");
	    s = q;
	    for (; r = q, q = r->left;);
	    if (r == s) {
		r->left = s = p->left;
		s->parent = r;
		if (r->parent = s = p->parent) {
		    if (p == s->left) {
			s->left = r;
		    } else {
			s->right = r;
		    }
		} else {
		    free_tree = r;
		}
		r->balance = p->balance;
		p = r;
		goto balance_right;
	    } else {
		t = r->parent;
		if (t->left = s = r->right) {
		    s->parent = t;
		}
		r->balance = p->balance;
		r->left = s = p->left;
		s->parent = r;
		r->right = s = p->right;
		s->parent = r;
		if (r->parent = s = p->parent) {
		    if (p == s->left) {
			s->left = r;
		    } else {
			s->right = r;
		    }
		} else {
		    free_tree = r;
		}
		p = t;
		goto balance_left;
	    }
	} else {		/* no right child, but left child */
	    /*
	     * We set up the free list in a way so that there will remain at
	     * least two nodes, and the avl property ensures that the left
	     * child is a leaf ==> there is a parent
	     */
	    fake("no right child, but left child");
	    s = p;
	    p = s->parent;
	    r = s->left;
	    r->parent = p;
	    if (s == p->left) {
		p->left = r;
		goto balance_left;
	    } else {
		p->right = r;
		goto balance_right;
	    }
	}
    } else {			/* no left child */
	/*
	 * We set up the free list in a way so that there is a node left of
	 * all used nodes, so there is a parent
	 */
	fake("no left child");
	s = p;
	p = s->parent;
	if (q = r = s->right) {
	    r->parent = p;
	}
	if (s == p->left) {
	    p->left = r;
	    goto balance_left;
	} else {
	    p->right = r;
	    goto balance_right;
	}
    }
  balance_q:
    r = p;
    p = q;
    if (r == p->right) {
	balance_t b;

      balance_right:
	b = p->balance;
	if (b > 0) {
	    p->balance = 0;
	    if (q = p->parent)
		goto balance_q;
	    return;
	} else if (b < 0) {
	    r = p->left;
	    b = r->balance;
	    if (b <= 0) {
		/* R-Rotation */
#ifdef DEBUG_AVL
		fake("R-Rotation.");
		printf("r->balance: %d\n", r->balance);
#endif
		if (p->left = s = r->right) {
		    s->parent = p;
		}
		r->right = p;
		s = p->parent;
		p->parent = r;
		b += 1;
		r->balance = b;
		b = -b;
#ifdef DEBUG_AVL
		printf("node r: %x\n", r);
		printf("r->balance: %d\n", r->balance);
		printf("node p: %x\n", p);
		p->balance = b;
		printf("p->balance: %d\n", p->balance);
		printf("r-height: %d\n", check_avl(r->parent, r));
#endif
		if (r->parent = s) {
		    if (p->balance = b) {
			if (p == s->left) {
			    s->left = r;
			    return;
			} else {
			    s->right = r;
			    return;
			}
		    }
		    if (p == s->left) {
			fake("left from parent");
			goto balance_left_s;
		    } else {
			fake("right from parent");
			p = s;
			p->right = r;
			goto balance_right;
		    }
		}
		p->balance = b;
		free_tree = r;
		return;
	    } else {		/* r->balance == +1 */
		/* LR-Rotation */
		balance_t b2;

		fake("LR-Rotation.");
		t = r->right;
		b = t->balance;
		if (p->left = s = t->right) {
		    s->parent = p;
		}
		if (r->right = s = t->left) {
		    s->parent = r;
		}
		t->left = r;
		t->right = p;
		r->parent = t;
		s = p->parent;
		p->parent = t;
#ifdef NO_BARREL_SHIFT
		b = -b;
		b2 = b >> 1;
		r->balance = b2;
		b -= b2;
		p->balance = b;
#else
		b2 = (unsigned char) b >> 7;
		p->balance = b2;
		b2 = -b2 - b;
		r->balance = b2;
#endif
		t->balance = 0;
#ifdef DEBUG_AVL
		printf("t-height: %d\n", check_avl(t->parent, t));
#endif
		if (t->parent = s) {
		    if (p == s->left) {
			p = s;
			s->left = t;
			goto balance_left;
		    } else {
			p = s;
			s->right = t;
			goto balance_right;
		    }
		}
		free_tree = t;
		return;
	    }
	} else {		/* p->balance == 0 */
	    p->balance = -1;
	    return;
	}
    } else {			/* r == p->left */
	balance_t b;

	goto balance_left;
      balance_left_s:
	p = s;
	s->left = r;
      balance_left:
	b = p->balance;
	if (b < 0) {
	    p->balance = 0;
	    if (q = p->parent)
		goto balance_q;
	    return;
	} else if (b > 0) {
	    r = p->right;
	    b = r->balance;
	    if (b >= 0) {
		/* L-Rotation */
#ifdef DEBUG_AVL
		fake("L-Rotation.");
		printf("r->balance: %d\n", r->balance);
#endif
		if (p->right = s = r->left) {
		    s->parent = p;
		}
		fake("subtree relocated");
		r->left = p;
		s = p->parent;
		p->parent = r;
		b -= 1;
		r->balance = b;
		b = -b;
#ifdef DEBUG_AVL
		fake("balances calculated");
		printf("node r: %x\n", r);
		printf("r->balance: %d\n", r->balance);
		printf("node p: %x\n", p);
		p->balance = b;
		printf("p->balance: %d\n", p->balance);
		printf("r-height: %d\n", check_avl(r->parent, r));
#endif
		if (r->parent = s) {
		    if (p->balance = b) {
			if (p == s->left) {
			    s->left = r;
			    return;
			} else {
			    s->right = r;
			    return;
			}
		    }
		    if (p == s->left) {
			fake("left from parent");
			goto balance_left_s;
		    } else {
			fake("right from parent");
			p = s;
			p->right = r;
			goto balance_right;
		    }
		}
		p->balance = b;
		free_tree = r;
		return;
	    } else {		/* r->balance == -1 */
		/* RL-Rotation */
		balance_t b2;

		fake("RL-Rotation.");
		t = r->left;
		b = t->balance;
		if (p->right = s = t->left) {
		    s->parent = p;
		}
		if (r->left = s = t->right) {
		    s->parent = r;
		}
		t->right = r;
		t->left = p;
		r->parent = t;
		s = p->parent;
		p->parent = t;
#ifdef NO_BARREL_SHIFT
		b = -b;
		b2 = b >> 1;
		p->balance = b2;
		b -= b2;
		r->balance = b;
#else
		b2 = (unsigned char) b >> 7;
		r->balance = b2;
		b2 = -b2 - b;
		p->balance = b2;
#endif
		t->balance = 0;
		if (t->parent = s) {
		    if (p == s->left) {
			p = s;
			s->left = t;
			goto balance_left;
		    } else {
			s->right = t;
			p = s;
			goto balance_right;
		    }
		}
		free_tree = t;
		return;
	    }
	} else {		/* p->balance == 0 */
	    p->balance++;
	    return;
	}
    }
}

static void add_to_free_list (u * ptr)
{
    u size;
    free_block_t *p, *q, *r;

    /*
     * When there is a distinction between data and address registers and/or
     * accesses, gcc will choose data type for q, so an assignmnt to q will
     * faciliate branching
     */

    fake((do_check_avl(), "add_to_free_list called"));
    size = *ptr & MASK;
#ifdef DEBUG_AVL
    printf("size:%d\n", size);
#endif
    q = (free_block_t *) size;	/* this assignment is a hint for
					 * register choice */
    r = (free_block_t *) (ptr + 1);
    count_up(large_free_stat, size << 2);
    q = free_tree;
    for (;; /* p = q */ ) {
	p = (free_block_t *) q;
#ifdef DEBUG_AVL
	printf("checked node size %d\n", p->size);
#endif
	if (size < p->size) {
	    if (q = p->left) {
		continue;
	    }
	    fake("add left");
	    p->left = r;
	    break;
	} else {		/* >= */
	    if (q = p->right) {
		continue;
	    }
	    fake("add right");
	    p->right = r;
	    break;
	}
    }
    r->size = size;
    r->parent = p;
    r->left = 0;
    r->right = 0;
    r->balance = 0;
#ifdef DEBUG_AVL
    fake("built new leaf.");
    printf("p->balance:%d\n", p->balance);
#endif
    do {
	free_block_t *s;

	if (r == p->left) {
	    balance_t b;

	    if (!(b = p->balance)) {
#ifdef DEBUG_AVL
		printf("p->size: %d\n", p->size);
		printf("p->balance: %d\n", p->balance);
		printf("p->right-h: %d\n", check_avl(p, p->right));
		printf("p->left -h: %d\n", check_avl(p, p->left));
		fake("growth propagation from left side");
#endif
		p->balance = -1;
	    } else if (b < 0) {
#ifdef DEBUG_AVL
		printf("p->balance:%d\n", p->balance);
#endif
		if (r->balance < 0) {
		    /* R-Rotation */
		    fake("R-Rotation");
		    if (p->left = s = r->right) {
			s->parent = p;
		    }
		    r->right = p;
		    p->balance = 0;
		    r->balance = 0;
		    s = p->parent;
		    p->parent = r;
		    if (r->parent = s) {
			if (s->left == p) {
			    s->left = r;
			} else {
			    s->right = r;
			}
		    } else {
			free_tree = r;
		    }
		} else {	/* r->balance == +1 */
		    /* LR-Rotation */
		    balance_t b2;
		    free_block_t *t = r->right;

#ifdef DEBUG_AVL
		    fake("LR-Rotation");
		    printf("t = %x\n", t);
		    printf("r->balance:%d\n", r->balance);
#endif
		    if (p->left = s = t->right) {
			s->parent = p;
		    }
		    fake("relocated right subtree");
		    t->right = p;
		    if (r->right = s = t->left) {
			s->parent = r;
		    }
		    fake("relocated left subtree");
		    t->left = r;
		    b = t->balance;
#ifdef NO_BARREL_SHIFT
		    b = -b;
		    b2 = b >> 1;
		    r->balance = b2;
		    b -= b2;
		    p->balance = b;
#else
		    b2 = (unsigned char) b >> 7;
		    p->balance = b2;
		    b2 = -b2 - b;
		    r->balance = b2;
#endif
		    t->balance = 0;
		    fake("balances calculated");
		    s = p->parent;
		    p->parent = t;
		    r->parent = t;
		    if (t->parent = s) {
			if (s->left == p) {
			    s->left = t;
			} else {
			    s->right = t;
			}
		    } else {
			free_tree = t;
		    }
#ifdef DEBUG_AVL
		    printf("p->balance:%d\n", p->balance);
		    printf("r->balance:%d\n", r->balance);
		    printf("t->balance:%d\n", t->balance);
		    fake((do_check_avl(), "LR-Rotation completed."));
#endif
		}
		break;
	    } else {		/* p->balance == +1 */
		p->balance = 0;
		fake("growth of left side balanced the node");
		break;
	    }
	} else {		/* r == p->right */
	    balance_t b;

	    if (!(b = p->balance)) {
		fake("growth propagation from right side");
		p->balance++;
	    } else if (b > 0) {
		if (r->balance > 0) {
		    /* L-Rotation */
		    fake("L-Rotation");
		    if (p->right = s = r->left) {
			s->parent = p;
		    }
		    r->left = p;
		    p->balance = 0;
		    r->balance = 0;
		    s = p->parent;
		    p->parent = r;
		    if (r->parent = s) {
			if (s->left == p) {
			    s->left = r;
			} else {
			    s->right = r;
			}
		    } else {
			free_tree = r;
		    }
		} else {	/* r->balance == -1 */
		    /* RL-Rotation */
		    balance_t b2;
		    free_block_t *t = r->left;

#ifdef DEBUG_AVL
		    fake("RL-Rotation");
		    printf("t = %x\n", t);
		    printf("r->balance:%d\n", r->balance);
#endif
		    if (p->right = s = t->left) {
			s->parent = p;
		    }
		    fake("relocated left subtree");
		    t->left = p;
		    if (r->left = s = t->right) {
			s->parent = r;
		    }
		    fake("relocated right subtree");
		    t->right = r;
		    b = t->balance;
#ifdef NO_BARREL_SHIFT
		    b = -b;
		    b2 = b >> 1;
		    p->balance = b2;
		    b -= b2;
		    r->balance = b;
#else
		    b2 = (unsigned char) b >> 7;
		    r->balance = b2;
		    b2 = -b2 - b;
		    p->balance = b2;
#endif
		    t->balance = 0;
		    s = p->parent;
		    p->parent = t;
		    r->parent = t;
		    if (t->parent = s) {
			if (s->left == p) {
			    s->left = t;
			} else {
			    s->right = t;
			}
		    } else {
			free_tree = t;
		    }
		    fake("RL-Rotation completed.");
		}
		break;
	    } else {		/* p->balance == -1 */
#ifdef DEBUG_AVL
		printf("p->balance: %d\n", p->balance);
		printf("p->right-h: %d\n", check_avl(p, p->right));
		printf("p->left -h: %d\n", check_avl(p, p->left));
#endif
		p->balance = 0;
		fake("growth of right side balanced the node");
		break;
	    }
	}
	r = p;
	p = p->parent;
    } while (q = p);
    fake((do_check_avl(), "add_to_free_list successful"));
}

#else				/* FIT_STYLE_FAST_FIT */

void show_free_list()
{
    u *p;

    p = free_list;
    while (p) {
	show_block(p);
	p = l_next_ptr(p);
    }
    printf("\n");
}

t_stat large_free_stat;
void remove_from_free_list (u * ptr)
{
    count_back(large_free_stat, (*ptr & MASK) << 2);

    if (l_prev_ptr(ptr))
	l_next_ptr(l_prev_ptr(ptr)) = l_next_ptr(ptr);
    else
	free_list = l_next_ptr(ptr);

    if (l_next_ptr(ptr))
	l_prev_ptr(l_next_ptr(ptr)) = l_prev_ptr(ptr);
}

void add_to_free_list (u * ptr)
{
    extern int puts();

    count_up(large_free_stat, (*ptr & MASK) << 2);

#ifdef DEBUG
    if (free_list && l_prev_ptr(free_list))
	puts("Free list consistency error.");
#endif

    l_next_ptr(ptr) = free_list;
    if (free_list)
	l_prev_ptr(free_list) = ptr;
    l_prev_ptr(ptr) = 0;
    free_list = ptr;
}
#endif				/* FIT_STYLE_FAST_FIT */

static void build_block (	/* build a properly annotated unalloc block */
			                  u * ptr, u size)
{
    u tmp;

    tmp = (*ptr & PREV_BLOCK) | size;
    *(ptr + size - 1) = size;
    *(ptr) = tmp;		/* mark this block as free */
    *(ptr + size) &= ~PREV_BLOCK;	/* unmark previous block */
}

static void mark_block (	/* mark this block as allocated */
			                u * ptr)
{
    *l_next_block(ptr) |= PREV_BLOCK;
    *ptr |= THIS_BLOCK;
}

/*
 * It is system dependent how sbrk() aligns data, so we simpy use brk()
 * to insure that we have enough.
 */
t_stat sbrk_stat;
static char *esbrk (u size)
{
#ifdef SBRK_OK
#ifdef NeXT

    void *addr = NULL;
    static void *current_break = NULL;
    static int anywhere = FALSE;
    kern_return_t ret;

    /*
     * try to extend current break
     */
    addr = current_break;
    ret = vm_allocate(task_self(), (vm_address_t *) & addr, size, anywhere);
    if (ret != KERN_SUCCESS && (anywhere == FALSE)) {
	/*
	 * allocate anywhere
	 */
	anywhere = TRUE;
	ret = vm_allocate(task_self(), (vm_address_t *) & addr, size, TRUE);
	if (ret != KERN_SUCCESS)
	    return (NULL);
    }
    count_up(sbrk_stat, size);
    current_break = (void *) ((char *) addr + size);
    return (addr);

#else

#ifndef linux
    extern char *sbrkx();
#endif				/* linux */

    static char *current_break = 0;

    if (current_break == 0)
	current_break = sbrkx(0);
    if (brk(current_break + size) == -1)
	return 0;

    count_up(sbrk_stat, size);
    current_break += size;
    return current_break - size;

#endif				/* NeXT */
#else				/* not SBRK_OK */

    count_up(sbrk_stat, size);
    return (char *)malloc(size);

#endif				/* SBRK_OK */
}

t_stat large_alloc_stat;
static char *large_malloc (u size, int force_more)
{
    u real_size;
    u *ptr;

    fake("large_malloc called");
#ifdef LARGE_TRACE
    printf("request:%d.", size);
#endif
    size = (size + 7) >> 2;	/* plus overhead */
    count_up(large_alloc_stat, size << 2);

  retry:
    ptr = 0;
    if (!force_more) {
#ifdef FIT_STYLE_FAST_FIT

	free_block_t *p, *q, *r;
	u minsplit;
	u tempsize;

	ptr++;
	minsplit = size + SMALL_BLOCK_MAX + 1;
	q = free_tree;
	for (;;) {
	    p = q;
#ifdef DEBUG_AVL
	    printf("checked node size %d\n", p->size);
#endif
	    tempsize = p->size;
	    if (minsplit < tempsize) {
		ptr = (u *) p;	/* remember this fit */
		if (q = p->left) {
		    continue;
		}
		/* We don't need that much, but that's the best fit we have */
		break;
	    } else if (size > tempsize) {
		if (q = p->right) {
		    continue;
		}
		break;
	    } else {		/* size <= tempsize <= minsplit */
		if (size == tempsize) {
		    ptr = (u *) p;
		    break;
		}
		/* size < tempsize */
		if (q = p->left) {
		    r = p;
		    /*
		     * if r is used in the following loop instead of p, gcc
		     * will handle q very inefficient throughout the function
		     * large_malloc()
		     */
		    for (;;) {
			p = q;
			tempsize = p->size;
			if (size < tempsize) {
			    if (q = p->left) {
				continue;
			    }
			    break;
			} else if (size > tempsize) {
			    if (q = p->right) {
				continue;
			    }
			    break;
			} else {
			    ptr = (u *) p;
			    goto found_fit;
			}
		    }
		    p = r;
		}
		tempsize = p->size;
		if (minsplit > tempsize) {
		    if (q = p->right) {
			for (;;) {
			    p = q;
			    tempsize = p->size;
			    if (minsplit <= tempsize) {
				ptr = (u *) p;	/* remember this fit */
				if (q = p->left) {
				    continue;
				}
				break;
			    } else {	/* minsplit > tempsize */
				if (q = p->right) {
				    continue;
				}
				break;
			    }
			}	/* end inner for */
			break;
		    }
		    break;	/* no new fit */
		}
		/* minsplit == tempsize  ==> best non-exact fit */
		ptr = (u *) p;
		break;
	    }
	}			/* end outer for */
      found_fit:
	ptr--;
#else				/* FIT_STYLE */
	u best_size;
	u *first, *best;

#ifdef LARGE_TRACE
	u search_length = 0;

#endif

	first = best = 0;
	best_size = MASK;
	ptr = free_list;

	while (ptr) {
	    u tempsize;

#ifdef LARGE_TRACE
	    search_length++;
#endif
	    /* Perfect fit? */
	    tempsize = *ptr & MASK;
	    if (tempsize == size) {
		best = first = ptr;
		break;
		/* always accept perfect fit */
	    }
	    /* does it really even fit at all */
	    if (tempsize >= size + SMALL_BLOCK_MAX + 1) {
		/* try first fit */
		if (!first) {
		    first = ptr;
		    if (fit_style == FIRST_FIT)
			break;
		    /* just use this one! */
		}
		/* try best fit */
		tempsize -= size;
		if (tempsize > 0 && tempsize <= best_size) {
		    best = ptr;
		    best_size = tempsize;
		}
	    }
	    ptr = l_next_ptr(ptr);
	}			/* end while */

#ifdef LARGE_TRACE
	printf("search length %d\n", search_length);
#endif
	if (fit_style == BEST_FIT)
	    ptr = best;
	else
	    ptr = first;
	/* FIRST_FIT and HYBRID both leave it in first */

#endif				/* FIT_STYLE */
    }				/* end of  if (!force_more) */
    if (!ptr) {			/* no match, allocate more memory */
	u chunk_size, block_size;

	block_size = size * SIZEOF_INT;
	if (force_more || (block_size > CHUNK_SIZE))
	    chunk_size = block_size;
	else
	    chunk_size = CHUNK_SIZE;

#ifdef SBRK_OK
	if (!start_next_block) {
	    count_up(large_alloc_stat, SIZEOF_INT);
	    start_next_block = (u *) esbrk(SIZEOF_INT);
	    if (!start_next_block)
		fatal("Couldn't malloc anything");
	    *(start_next_block) = PREV_BLOCK;
	    fake("Allocated little fake block");
	}
	ptr = (u *) esbrk(chunk_size);
#else				/* not SBRK_OK */
	ptr = (u *) esbrk(chunk_size + SIZEOF_INT);
#endif				/* SBRK_OK */
	if (ptr == 0) {
	    extern char *reserved_area;
	    extern int slow_shut_down_to_do;
	    static int going_to_exit = 0;
	    static char mess1[] = "Temporary out of MEMORY. Freeing reserve.\n";
	    static char mess2[] = "Totally out of MEMORY.\n";

	    if (going_to_exit)
		exit(3);
	    if (reserved_area) {
		smalloc_free(reserved_area);
		reserved_area = 0;
		write(1, mess1, sizeof(mess1) - 1);
		slow_shut_down_to_do = 6;
		force_more = 0;
		goto retry;
	    }
	    if (force_more) {
		force_more = 0;
		goto retry;
	    }
	    going_to_exit = 1;
	    write(1, mess2, sizeof(mess2) - 1);
	    (void) dump_trace(0);
	    exit(2);
	}
#ifdef SBRK_OK
	ptr -= 1;		/* overlap old memory block */
#else				/* not SBRK_OK */
	if (start_next_block == ptr) {
	    ptr -= 1;		/* overlap old memory block */
	    chunk_size += SIZEOF_INT;
	} else
	    *ptr = PREV_BLOCK;
	start_next_block = (u *) ((char *) ptr + chunk_size);
#endif				/* SBRK_OK */
	block_size = chunk_size / SIZEOF_INT;

	/* configure header info on chunk */

	build_block(ptr, block_size);
	if (force_more)
	    fake("Build little block");
	else
	    fake("Built memory block description.");
	*l_next_block(ptr) = THIS_BLOCK;
	add_to_free_list(ptr);
    }				/* end of creating a new chunk */
    remove_from_free_list(ptr);
    real_size = *ptr & MASK;

    if (real_size - size) {
	/* split block pointed to by ptr into two blocks */
	build_block(ptr + size, real_size - size);
	fake("Built empty block");
	/*
	 * When we allocate a new chunk, it might differ very slightly in
	 * size from the desired size.
	 */
	if (real_size - size >= SMALL_BLOCK_MAX + 1) {
	    add_to_free_list(ptr + size);
	} else {
	    mark_block(ptr + size);
	}
	build_block(ptr, size);
    }
    mark_block(ptr);
    fake("built allocated block");
    return (char *) (ptr + 1);
}

static void large_free (char * ptr)
{
    u size, *p;

    p = (u *) ptr;
    p -= 1;
    size = *p & MASK;
    count_back(large_alloc_stat, (size << 2));

    if (!(*(p + size) & THIS_BLOCK)) {
	remove_from_free_list(p + size);
	size += (*(p + size) & MASK);
	*p = (*p & PREV_BLOCK) | size;
    }
    if (l_prev_free(p)) {
	remove_from_free_list(l_prev_block(p));
	size += (*l_prev_block(p) & MASK);
	p = l_prev_block(p);
    }
    build_block(p, size);

    add_to_free_list(p);
}

POINTER CDECL smalloc_realloc (POINTER p, size_t size)
{
    unsigned *q, old_size;
    char *t;

    q = (unsigned *) p;

#if MALLOC_ALIGN > 4
    while (!(old_size = *--q));
    old_size = ((old_size & MASK) - 1) * SIZEOF_INT;
#else
    --q;
    old_size = ((*q & MASK) - 1) * SIZEOF_INT;
#endif
    if (old_size >= size)
	return p;

    t = smalloc_malloc(size);
    if (t == 0)
	return (char *) 0;

    memcpy(t, p, old_size);
    smalloc_free(p);
    return t;
}

static int resort_free_list()
{
    return 0;
}
#ifdef DO_MSTATS
#define dump_stat(str,stat) outbuf_addv(ob, str,stat.counter,stat.size)
void show_mstats (outbuffer_t * ob, char * s)
{
    outbuf_addv(ob, "Memory allocation statistics %s\n", s);
    outbuf_add(ob, "Type                   Count      Space (bytes)\n");
    dump_stat("sbrk requests:     %8d        %10d (a)\n", sbrk_stat);
    dump_stat("large blocks:      %8d        %10d (b)\n", large_alloc_stat);
    dump_stat("large free blocks: %8d        %10d (c)\n\n", large_free_stat);
    dump_stat("small chunks:      %8d        %10d (d)\n", small_chunk_stat);
    dump_stat("small blocks:      %8d        %10d (e)\n", small_alloc_stat);
    dump_stat("small free blocks: %8d        %10d (f)\n", small_free_stat);
    outbuf_addv(ob,
       "unused from current chunk          %10d (g)\n\n", unused_size);
    outbuf_addv(ob,
    "    Small blocks are stored in small chunks, which are allocated as\n");
    outbuf_addv(ob,
    "large blocks.  Therefore, the total large blocks allocated (b) plus\n");
    outbuf_addv(ob,
    "the large free blocks (c) should equal total storage from sbrk (a).\n");
    outbuf_addv(ob,
    "Similarly, (e) + (f) + (g) equals (d).  The total amount of storage\n");
    outbuf_addv(ob,
    "wasted is (c) + (f) + (g); the amount allocated is (b) - (f) - (g).\n");
}
#endif

/*
 * calloc() is provided because some stdio packages uses it.
 */
POINTER CDECL smalloc_calloc (size_t nelem, size_t sizel)
{
    char *p;

    if (nelem == 0 || sizel == 0)
	return 0;
    p = smalloc_malloc(nelem * sizel);
    if (p == 0)
	return 0;
    (void) memset(p, '\0', nelem * sizel);
    return p;
}

#ifdef DEBUG
/*
 * Functions below can be used to debug malloc.
 */

static void walk_new_small_malloced(func)
    void (*func) (POINTER, int);
{
    int i;
    u *p, *q;

    for (i = 0; i < SMALL_BLOCK_MAX; i++) {
	for (p = sfltable[i]; p; p = *(u **) (p + 1)) {
	    *s_size_ptr(p) |= NO_REF;
	}
    }
    if (unused_size)
	*next_unused = 0;
    for (p = last_small_chunk; p; p = *(u **) p) {
	u *end = p - 1 + (p[-1] & MASK);

	debug_message("scanning chunk %x, end %x\n", (u) (p - 1), (u) end);
	for (q = p + 1; q < end;) {
	    u size = *s_size_ptr(q);

	    if (!size)
		break;
	    if (!(size & NO_REF)) {
		(*func) ((char *) s_next_ptr(q), (size & MASK) << 2);
		*s_size_ptr(q) |= NO_REF;
	    }
	    q += size & MASK;
	}
    }
    for (i = 0; i < SMALL_BLOCK_MAX; i++) {
	for (p = sfltable[i]; p; p = *(u **) (p + 1)) {
	    *s_size_ptr(p) &= ~NO_REF;
	}
    }
}
#endif

#if 0

int debug_smalloc = 0;

/*
 * Verify that the free list is correct. The upper limit compared to
 * is very machine dependant.
 */
verify_sfltable()
{
    u *p;
    int i, j;
    extern int end;

    if (!debug_smalloc)
	return;
    if (unused_size > SMALL_CHUNK_SIZE)
	apa();
    for (i = 0; i < SMALL_BLOCK_MAX; i++) {
	for (j = 0, p = sfltable[i]; p; p = *(u **) (p + 1), j++) {
	    if (p < (u *) & end || p > (u *) 0xfffff)
		apa();
	    if (*p - 2 != i)
		apa();
	}
	if (p >= next_unused && p < next_unused + (unused_size >> 2))
	    apa();
    }
    p = free_list;
    while (p) {
	if (p >= next_unused && p < next_unused + (unused_size >> 2))
	    apa();
	p = l_next_ptr(p);
    }
}

verify_free (u * ptr)
{
    u *p;
    int i, j;

    if (!debug_smalloc)
	return;
    for (i = 0; i < SMALL_BLOCK_MAX; i++) {
	for (j = 0, p = sfltable[i]; p; p = *(u **) (p + 1), j++) {
	    if (*p - 2 != i)
		apa();
	    if (ptr >= p && ptr < p + *p)
		apa();
	    if (p >= ptr && p < ptr + *ptr)
		apa();
	    if (p >= next_unused && p < next_unused + (unused_size >> 2))
		apa();
	}
    }

    p = free_list;
    while (p) {
	if (ptr >= p && ptr < p + (*p & MASK))
	    apa();
	if (p >= ptr && p < ptr + (*ptr & MASK))
	    apa();
	if (p >= next_unused && p < next_unused + (unused_size >> 2))
	    apa();
	p = l_next_ptr(p);
    }
    if (ptr >= next_unused && ptr < next_unused + (unused_size >> 2))
	apa();
}

apa()
{
    int i;

    i / 0;
}

static char *ref;
test_malloc (char * p)
{
    if (p == ref)
	printf("Found 0x%x\n", p);
}

#endif				/* 0 (never) */
