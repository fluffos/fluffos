/* 92/04/18 - cleaned up in accordance with ./src/style.guidelines */

#include <stdio.h>
#include <string.h>
#ifdef sun
#include <alloca.h>
#endif
#include "config.h"
#include "lint.h"
#include "interpret.h"
#include "object.h"
#include "wiz_list.h"
#include "regexp.h"

#include "debug.h" /* added by Truilkan */

/*
   this INLINE def should be moved into some global include file so
   that the same one can be used by all source modules
*/
#if defined(__GNUC__) && !defined(lint)
#define INLINE inline
#else
#define INLINE
#endif

extern int sameval PROT((struct svalue *, struct svalue *));
extern int d_flag;
extern struct object *master_ob,
                     *current_object;
extern struct svalue const0u;
extern int max_array_size;

INLINE void node_copy PROT((struct node *, struct mapping *));

int num_mappings = 0;
int total_mapping_size = 0;
int aggregate = 0;

/*
 * LPC mapping (associative arrays) package.  Contains routines for
 * easy value and lvalue manipulation.  Will grow in the near future.
 *
 * original version for LPC-A - written by Whiplash (JTR)
 * - some enhancements by Truilkan@TMI
 */
 
/*
 * About the implementation:
 *  Naturally, it was essential to allow mappings to grow and/or
 *  shrink without having to create a new mapping.  Hence the
 *  tree structure.  An earlier implementation used a handle
 *  to a hash table.  When it came time to upsize, it doubled
 *  the size and allocated a new hash table to which everything
 *  was copied.  But the problem with that was that LPC lvalues
 *  need the pointers to values within data structures to remain
 *  unchanged, usually for the entire lifetime of the data
 *  structure.  Trees allow this to be done.
 *  [Actually hash tables could be used without running into the above
 *  mentioned difficulties if you used double indirection.]
 *
 *  I sort of solved the memory fragmentation problem (and malloc overhead
 *  problem) by making the initial mapping a contiguous block in memory.
 *
 *  Performance:
 *  o Best as an aggregate (or restored from an object - same
 *    thing).  It gives the best memory usage, about 50% more
 *    than an array would take up.
 *  o It's ok at insertion, good at 'deletion'  (mainly because
 *    it doesn't downsize), good at retrieval, good at storage.
 *  o It's intended to be friendly to what's probably the most
 *    common usage, a glorified lookup table.  It'd be really
 *    suited to a good skill package, esp. since indexed mappings
 *    can be lvalues:
 *      skills["fighting"] += 30;
 *    (Ooops, sorry for mentioning an actual application for
 *     this code :))
 *  o These beat out arrays in terms of speed and efficiency,
 *    PROVIDED you're indexing off of non-integer values.
 *    (you gain the efficiency because you don't have to write LPC code
 *    for converting the indices to integers, nyet?)
 *  o They can also be used for sparse arrays, sacrificing
 *    a tiny amount of speed (which you'd lose anyway in the
 *    LPC implementation of sparse arrays), and gaining quite
 *    a bit of memory.
 */

/* free_node */

INLINE void
free_node(n, m)
struct node *n;
struct mapping *m;
{
	debug(1,("mapping.c: free_node begin\n"));
	if (n->right) free_node(n->right, m); 
	if (n->left) free_node(n->left, m);
	free_svalue(&n->values[1]);
	free_svalue(&n->values[0]);
	/* if in the original block then don't free it */
	if ((n >= m->block) && (n < (m->block + m->orig_size)))
		return; 
	total_mapping_size -= sizeof(struct node);
	if (m->user)
		m->user->size_array -= 2;
	FREE((char *) n);
	debug(1,("mapping.c: free_node end\n"));
}

/* free_mapping */
 
void
free_mapping(m)
struct mapping *m;
{
	debug(64,("mapping.c: free_mapping begin\n"));
	m->ref--;
	if (m->ref > 0) return;
	num_mappings--;
	if (m->nod)
		free_node(m->nod, m);
	total_mapping_size -= sizeof(struct mapping)
		+ m->orig_size * sizeof(struct node);
	if (m->user)
		m->user->size_array -= m->orig_size * 2;
	FREE((char *) m);
	debug(1,("mapping.c: free_mapping end\n"));
}

/* allocate_mapping */
 
INLINE struct mapping *
allocate_mapping(n)
int n;
{
	struct mapping *newmap;

	debug(64,("mapping.c: allocate_mapping begin\n"));
	newmap = ALLOC_MAPPING(n);
	total_mapping_size += sizeof(struct mapping) + n * sizeof(struct node);
	newmap->orig_size = n;
	/* do we need to add a ref to current_object->user ? -- Tru */
	newmap->user = current_object->user;
	if (newmap->user)
		newmap->user->size_array += n * 2;
	newmap->nod = (struct node *) 0;
	newmap->ref = 1;
	newmap->size = 0;
	/* here we only add 1 to newmap since we are using pointer arithmetic */
	newmap->block = (struct node *)(newmap + 1);
	newmap->cur = newmap->block;
	num_mappings++;
	debug(1,("mapping.c: allocate_mapping end\n"));
	return newmap;
}
 
/*
 * svalue_to_int: Converts an svalue into an integer index.
 * At an attempt to get good distribution so that our tree structure
 * doesn't get too imbalanced, the index will always be a
 * number between 0 and (arbitrary_prime - 1).
 */
#define A_PRIME 3217

int
svalue_to_int(v)
struct svalue *v;
{
   int i;

	debug(1,("mapping.c: svalue_to_int\n"));
	switch (v->type) {
	case T_NUMBER:
		i = v->u.number;
		if (i < 0) i = -i;
		return i % A_PRIME;
	case T_STRING:
		return hashstr(v->u.string, 12, A_PRIME);
	case T_OBJECT:
		return hashstr(v->u.ob->name, 12, A_PRIME);
	default:
	return A_PRIME;  /* so much for our precious distribution*/
	}
}

/*
 * node_find_in_mapping: Like find_for_insert(), but doesn't attempt
 * to add anything if a value is not found.  The pointer returned won't
 * necessarily have any meaningful value.
 */

struct node *
node_find_in_mapping(m, lv)
struct mapping *m;
struct svalue *lv;
{
	int i;
	struct node *n;
 
	debug(1,("mapping.c: find_in_mapping\n"));
	if (!(n = m->nod)) return (struct node *)0;
	i = svalue_to_int(lv);
	while (n) {
		int nodval; 

		nodval = svalue_to_int(&n->values[0]);
		if (i < nodval)
			n = n->left;  
		else {
			if (i == nodval && !n->deleted && sameval(&n->values[0], lv))
				return n;
			n = n->right;
		}
	}
	return (struct node *)0;
}

/*
   mapping_delete: delete an element from the mapping (actually just mark
   as deleted [available for reuse] since we are using a plain binary tree).
*/

void mapping_delete(m, lv)
struct mapping *m;
struct svalue *lv;
{
	struct node *n;

	n = node_find_in_mapping(m, lv);
	if (!n)
		return;

	debug(64,("mapping delete: found element\n"));
	free_svalue(&n->values[1]);
	m->size--;
	n->deleted = 1;
}
 
/*
 * find_for_insert: Tries to find an address at which an rvalue
 * can be inserted in a mapping.  This can also be used by the
 * microcode interpreter, to translate an expression <mapping>[index]
 * into an lvalue.
 */

struct svalue *
find_for_insert(m, lv)
struct mapping *m;
struct svalue *lv;
{
   int i, nodval;
   struct node *n, *newnode, *prev = (struct node *) 0;
 
	debug(1,("mapping.c: find_for_insert\n"));
	i = svalue_to_int(lv);
	n = m->nod;
	while (n) {
		nodval = svalue_to_int(&n->values[0]);
		if (i < nodval) {
			prev = n;
			n = n->left;
		} else /* if (i >= nodval) */ {
			if (i == nodval && (n->deleted || (sameval(lv, &n->values[0])))) {
				/* special case, we just replace rvalue. */
				if (aggregate && !n->deleted) {
				/* we're doing an aggregate initialization. the
				   agg. initialzation won't free this value... */ 
				free_svalue(&n->values[1]);
			}
			n->deleted = 0;
			return &n->values[1];
			}
			prev = n;
			n = n->right;
		}
	}
	if (m->size < m->orig_size) { /* Tru - 92/02/08 */
		newnode = m->cur;
		m->cur++;
	} else {
		if (m->user)
			m->user->size_array += 2;
		total_mapping_size += sizeof(struct node);
		newnode = (struct node *) xalloc(sizeof(struct node));
	}
	newnode->deleted = 0;
	newnode->left = (struct node *) 0;
	newnode->right = (struct node *) 0;
	assign_svalue_no_free(&newnode->values[0], lv);
	newnode->values[1].type = T_NUMBER;
	newnode->values[1].u.number = 0;
	if (prev) {
		if (i < nodval) /* we're to the left */
			prev->left = newnode;
		else
			prev->right = newnode;
	} else {
		m->nod = newnode;
	}
	m->size++; /* increment count of number of nodes in mapping */
	return &newnode->values[1];
}
 
/*
 * load_mapping_from_aggregate: Create a new mapping, loading from an
 * array of svalues. Format of data: LHS RHS LHS2 RHS2...
 */

struct mapping *
load_mapping_from_aggregate(sp, n)
struct svalue *sp;
int n;
{
	int i;
	struct mapping *m;
	struct svalue *v;
 
	debug(1,("mapping.c: load_mapping_from_aggregate begin\n"));
	aggregate = 1; /* indicate doing a load mapping from aggregate */
	m = allocate_mapping(n);
	for (i = 0; i < n; i += 2) {
		v = find_for_insert(m, sp + i);
		assign_svalue_no_free(v, sp + i + 1);
	}
	aggregate = 0; /* no longer doing a load mapping from aggregate */
	debug(1,("mapping.c: load_mapping_from_aggregate end\n"));
	return m;
}

struct svalue *
find_in_mapping(m, lv)
struct mapping *m;
struct svalue *lv;
{
   int i;
   struct node *n;
 
	n = node_find_in_mapping(m, lv);
	if (!n)
		return &const0u;
	return &n->values[1];
}

/*
   add_mapping: returns a new mapping that contains everything
   in two old mappings.
*/

struct mapping *
add_mapping(m1, m2)
struct mapping *m1, *m2;
{
	struct mapping *newmap;
 
	debug(1,("mapping.c: add_mapping begin\n"));
	newmap = allocate_mapping(m1->size + m2->size);
	/*
	  Note there's some memory wasted (not leaked, wasted).
      The second mapping may duplicate some or all of the
      lvalues in the first mapping.  Currently it doesn't
      look like it's worth the effort to save the memory. -JTR
	*/
	aggregate = 1; /* find_for_insert will be called from inside this module */
	if (m1->nod) node_copy(m1->nod, newmap);
	if (m2->nod) node_copy(m2->nod, newmap);
	debug(1,("mapping.c: add_mapping end\n"));
	aggregate = 0; /* find_for_insert may be called from outside this module */
	return newmap;
}

/* node_copy */
 
INLINE void
node_copy(src, dst)
struct node *src;
struct mapping *dst;
{
   struct svalue *v;

	debug(1,("mapping.c: node_copy\n"));
	if (!src->deleted) {
		v = find_for_insert(dst, &src->values[0]);
		assign_svalue_no_free(v, &src->values[1]);
	}
	if (src->left)
		node_copy(src->left, dst);
	if (src->right)
		node_copy(src->right, dst);
}


/* map_nodes */
 
void
map_nodes(nod, accp, cntp, func, ob, extra)
struct node *nod;
struct svalue *accp;
int *cntp;
struct object *ob;
char *func;
struct svalue *extra;
{
	struct svalue *ret;
 
	debug(1,("mapping.c: map_nodes\n"));
	push_svalue(&nod->values[1]);
	if (extra) {
		push_svalue(extra);
		ret = apply(func, ob, 2);
	} else
		ret = apply(func, ob, 1);
	if (ret && (ret->type != T_NUMBER || ret->u.number != 0)) {
		assign_svalue_no_free(&accp[*cntp++], &nod->values[0]);
		assign_svalue_no_free(&accp[*cntp++], ret);
	}
	if (nod->left)
		map_nodes(nod->left, accp, cntp, func, ob, extra);
	if (nod->right)
		map_nodes(nod->right, accp, cntp, func, ob, extra);
}
 
/*
   map_mapping: A lot of the efuns that work on arrays, such as
   filter_array(), should also work on mappings.
*/

struct mapping *
map_mapping(m, func, ob, extra)
struct mapping *m;
struct object *ob;
char *func;
struct svalue *extra;
{
	struct vector *v;
	int cnt = 0;
	struct mapping *newmap;
 
	debug(1,("mapping.c: map_mapping\n"));
	v = allocate_array(MAX_ARRAY_SIZE * 2);
	if (m->nod)
		map_nodes(m->nod, &v->item[0], &cnt, func, ob, extra);
	newmap = load_mapping_from_aggregate(&v->item[0], cnt);
	free_vector(v);
	return newmap;
}

/* count_match_nodes */

int
count_match_nodes(n1, m2, svp)
struct node *n1;
struct mapping *m2;
struct svalue *svp;
{
	struct svalue *newv;
	int x;

	debug(1,("mapping.c: count_match_nodes\n"));
	if (!n1)
		return 0;
	x = 0;
	if (!n1->deleted) {
		newv = find_in_mapping(m2, &n1->values[1]);
		if (!IS_UNDEFINED(newv)) {
			if (svp) {
				assign_svalue_no_free(svp++, &n1->values[0]);
				assign_svalue_no_free(svp++, newv);
				/* return 2 since each map entry consists of two elements */
				x = 2;
			}
		}
	}
	return x + count_match_nodes(n1->left, m2, svp) +
		count_match_nodes(n1->right, m2, svp);
}

/* compose_mapping */

struct mapping *
compose_mapping(m1, m2)
struct mapping *m1, *m2;
{
   int cnt;
   struct vector *v;
   struct mapping *newmap;

	debug(1,("mapping.c: compose_mapping\n"));
	/* First count the number of pairs that'd result. */
	cnt = count_match_nodes(m1->nod, m2, (struct svalue *) 0);
	v = allocate_array(cnt);
	(void) count_match_nodes(m1->nod, m2, &v->item[0]);
	newmap = load_mapping_from_aggregate(&v->item[0], cnt);
	free_vector(v);
	return newmap;
}

/* Some stuff to support mapping indices */
INLINE int
count_node_lhs(n)
struct node *n;
{
	int i = 0;

	if (n->left)
		i = count_node_lhs(n->left);
	if (n->right)
		i += count_node_lhs(n->right);
	debug(64,("n->deleted = %d\n",n->deleted));
	if (!n->deleted)
		i++;
	return i;
}

INLINE int 
copy_node_indices(nod, sp, n)
struct node *nod;
struct svalue *sp;
int n;
{
	int cnt, ret = 0;

	if (!nod)
		return 0;
	if (nod->left) {
		cnt = copy_node_indices(nod->left, sp, n);
		n -= cnt;
		ret += cnt;
	}
	if (nod->right) {
		cnt = copy_node_indices(nod->right, sp, n);
		n -= cnt;
		ret += cnt;
	}
	if (!n || nod->deleted) return ret;
	assign_svalue_no_free(sp+n-1, &nod->values[0]);
	return ++ret;
}

/* mapping_indices */

struct vector *
mapping_indices(m)
struct mapping *m;
{
	int siz;
	struct vector *v;

	siz = m->nod ? count_node_lhs(m->nod) : 0;
	if (siz > MAX_ARRAY_SIZE) siz = MAX_ARRAY_SIZE;
	v = allocate_array(siz); /* assigns ref count equal to 1 */
	v->ref--;  /* decrement ref count since F_ASSIGN increments it */
	debug(64,("siz = %d\n",siz));
	if (m->nod)
		(void) copy_node_indices(m->nod, &v->item[0], siz);
	return v;
}

/*
   combine_mapping: same as add_mapping except we first check to see if
   m1 has room for m2 so that we can try to get by without allocating a
   new mapping -- Tru@TMI.  This function was intended to be called
   from F_ADD_EQ (+=) in interpret.c but it isn't safe to use this until
   F_ASSIGN for mappings is changed to make a copy of the RHS instead
   of just making the LHS an alias for the RHS.
*/

struct mapping
*combine_mapping(m1, m2)
struct mapping *m1, *m2;
{
	if (m2->size < (m1->orig_size - m1->size)) {
		if (m2->nod)
			node_copy(m2->nod,m1);
		return m1;
	}
	else
		return add_mapping(m1,m2);
}
