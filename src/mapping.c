/* 92/04/18 - cleaned up in accordance with ./src/style.guidelines */

#include <stdio.h>
#include <string.h>
#ifdef sun
#include <alloca.h>
#endif
#include "config.h"
#include "lint.h"
#include "interpret.h"
#include "mapping.h"
#include "object.h"
#include "regexp.h"

#include "debug.h" /* added by Truilkan */

extern int sameval PROT((struct svalue *, struct svalue *));
extern int d_flag;
extern object_t *master_ob, *current_object;
extern struct svalue const0u, const0n;

INLINE void node_copy PROT((struct node *, struct mapping *));

int num_mappings = 0;
int total_mapping_size = 0;
int total_mapping_nodes = 0;

/* coeff taken from hash.c in Larry Wall's Perl package */

static char coeff[] = {
        61,59,53,47,43,41,37,31,29,23,17,13,11,7,3,1,
        61,59,53,47,43,41,37,31,29,23,17,13,11,7,3,1,
        61,59,53,47,43,41,37,31,29,23,17,13,11,7,3,1,
        61,59,53,47,43,41,37,31,29,23,17,13,11,7,3,1,
        61,59,53,47,43,41,37,31,29,23,17,13,11,7,3,1,
        61,59,53,47,43,41,37,31,29,23,17,13,11,7,3,1,
        61,59,53,47,43,41,37,31,29,23,17,13,11,7,3,1,
        61,59,53,47,43,41,37,31,29,23,17,13,11,7,3,1};

/*
 * LPC mapping (associative arrays) module.  Contains routines for
 * easy value and lvalue manipulation.
 *
 * Original binary tree version for LPCA written by one of the earliest MudOS
 * hackers.
 * - some enhancements by Truilkan@TMI
 * - rewritten for MudOS to use an extensible hash table implementation styled
 *   after the one Perl uses in hash.c - 92/07/08 - by Truilkan@TMI
 */

void mapping_too_large()
{
	error("Mapping exceeded maximum allowed size of %d.\n",MAX_MAPPING_SIZE);
}

/*
  growMap: based on hash.c:hsplit() from Larry Wall's Perl.

  growMap doubles the size of the hash table.  It is called when FILL_PERCENT
  of the buckets in the hash table contain values.  This routine is
  efficient since the entries in the table don't need to be rehashed (even
  though the entries are redistributed across the both halves of the hash
  table).
*/

void growMap(m)
struct mapping *m;
{
	int oldsize = m->table_size;
	int newsize = oldsize * 2;
	int theMask = newsize - 1;
	int i;
	struct node **a, **b, **eltp, *elt;

	if (newsize > MAX_TABLE_SIZE) {
		return;
	}
	/* resize the hash table to be twice the old size */
	a = (struct node **)
		DREALLOC(m->table, newsize * sizeof(struct node *), 72, "growMap");
	if (!a) {
		/*
		  We couldn't grow the hash table.  Rather than die, we just
		  accept the performance hit resulting from having an overfull table.
		*/
		return;
	}
	/* hash table doubles in size -- keep track of the memory used */
	total_mapping_size += (sizeof(struct node *) * oldsize);
	debug(1024,("mapping.c: growMap ptr = %x, size = %d\n", m, newsize));
	m->table = a;
	m->table_size = newsize;
	m->do_split = m->table_size * (unsigned)FILL_PERCENT / (unsigned)100;
	/* zero out the new storage area (2nd half of table) */
	memset(&a[oldsize], 0, oldsize * sizeof(struct node *));
	for (i = 0; i < oldsize; i++, a++) {
		if (!*a) {
			continue;
		}
		b = a + oldsize;  /* points to the ith element in the new half */
		for (eltp = a, elt = *a; elt; elt = *eltp) {
			/* if needs rehashed, then displace element by 'oldsize' buckets */
			if ((elt->hashval & theMask) != i) {  /* should move element? */
				*eltp = elt->next;
				elt->next = *b;
				if (!*b) {  /* *b used to be empty */
					m->filled++;
				}
				*b = elt;
				continue;
			} else {
				eltp = &elt->next;
			}
		}
		if (!*a) { /* everything that was in the bucket moved */
			m->filled--;
		}
	}
}

/*
  mapTraverse: iterate over the mapping, calling function 'func(elt, extra)'
  for each element 'elt'.  This is an attempt to encapsulate some of the
  specifics of the particular data structure being used so that it won't be
  so difficult to change the data structure if the need arises.
  -- Truilkan 92/07/19
*/

INLINE struct mapping *
mapTraverse(m, func, extra)
struct mapping *m;
int (*func) PROT((struct mapping *, struct node *, void *));
void *extra;
{
	struct node *elt, *nelt;
	int j;
	
	debug(128,("mapTraverse %x\n", m));
	for (j = 0; j < (int)m->table_size; j++) {
		for (elt = m->table[j]; elt; elt = nelt) {
			nelt = elt->next;
			if ((*func)(m, elt, extra))
				return m;
		}
	}
	return m;
}

/*
  doInsert: called from mapTraverse() (via copyMapping etc.)
*/

int
doInsert(m, elt, dest)
struct mapping *m;
struct node *elt;
struct mapping *dest;
{
	struct svalue *v;

	v = find_for_insert(dest, &elt->values[0], 1);
	if (v) {
		assign_svalue_no_free(v, &elt->values[1]);
	} else {
		mapping_too_large();
		return 1;
	}
	return 0;
}

/* doFree: called from mapTraverse() via free_mapping */

int
doFree(m, elt, extra)
struct mapping *m;
struct node *elt;
void *extra;
{
	free_svalue(&elt->values[1]);
	free_svalue(&elt->values[0]);
	total_mapping_size -= sizeof(struct node);
	FREE((char *) elt);
	return 0;
}
 
/* free_mapping */
 
void
free_mapping(m)
struct mapping *m;
{
	debug(64,("mapping.c: free_mapping begin, ptr = %x\n", m));
	m->ref--;
	/* some other object is still referencing this mapping */
	if (m->ref > 0)
		return;
	debug(1024,("mapping.c: actual free of %x\n", m));
	num_mappings--;
	total_mapping_size -= MAPSIZE(m->count);
	total_mapping_nodes -= m->count;
	add_array_size (&m->stats, - (m->count * 2));

	mapTraverse(m, doFree, NULL);

	debug(2048, ("in free_mapping: before table\n"));
	FREE((char *)m->table);
	total_mapping_size -= (sizeof(struct node *) * m->table_size);
	debug(2048, ("in free_mapping: after table\n"));
	FREE((char *) m);
	debug(2048, ("in free_mapping: after m\n"));
	debug(64,("mapping.c: free_mapping end\n"));
}

/* allocate_mapping(int n)
   
   call with n == 0 if you will be doing many deletions from the map.
   call with n == "approx. # of insertions you plan to do" if you won't be
   doing many deletions from the map.
*/
 
INLINE struct mapping *
allocate_mapping(n)
int n;
{
	struct mapping *newmap;
	int size, k;

	if (n > MAX_MAPPING_SIZE) {
		n = MAX_MAPPING_SIZE;
	}
	if (n < 0) {
		n = 0;
	}
	size = MAPSIZE(n);
	newmap = (struct mapping *)DXALLOC(size, 73, "allocate_mapping: 1");
	debug(1024,("mapping.c: allocate_mapping begin, newmap = %x\n", newmap));
	if (newmap == NULL) {
		n = 0;
		size = MAPSIZE(0);
		newmap = (struct mapping *)DXALLOC(size, 74, "allocate_mapping: 2");
	}
	total_mapping_size += size;
	for (k = MAP_HASH_TABLE_SIZE; k < n; k *= 2)
		;
	newmap->table_size = k;
	newmap->do_split = newmap->table_size * (unsigned)FILL_PERCENT /
		(unsigned)100;
	newmap->table = (struct node **)DXALLOC(sizeof(struct node *)
		* newmap->table_size, 75, "allocate_mapping: 3");
	total_mapping_size += (sizeof(struct node *) * newmap->table_size);
	/* zero out the hash table */
	memset(newmap->table, 0, newmap->table_size * sizeof(struct node *));
	newmap->ref = 1;
	newmap->count = 0;
	newmap->filled = 0;
#ifdef EACH
	newmap->eachObj = (struct object *)0;
	newmap->bucket = 0;
	newmap->elt = (struct node *)0;      /* must start at 0;see mapping_each */
#endif
	if (current_object) {
	  assign_stats (&newmap->stats, current_object);
	  add_array_size (&newmap->stats, n * 2);
	} else {
	  null_stats (&newmap->stats);
	}
	num_mappings++;
	debug(64,("mapping.c: allocate_mapping end\n"));
	return newmap;
}

/*
  copyMapping: make a copy of a mapping
  This could be made more efficient by avoiding the rehash caused
  by find_for_insert in doInsert (since we already know to what its going
  to hash).
*/

INLINE struct mapping *
copyMapping(m)
struct mapping *m;
{
	struct mapping *newmap;

	newmap = allocate_mapping(0);
	mapTraverse(m, (int (*)())doInsert, newmap);
	return newmap;
}

/*
   mapHashstr: based on code from hstore in Larry Wall's Perl package (hash.c)

   I use this hash function instead of hash.c:hashstr() because the scheme
   we use for growing the hash table requires the table size to be a power
   of 2 and I don't that the other hash function performs well for non-
   prime moduli.
*/

INLINE int
mapHashstr(key)
char *key;
{
	int hash, i;
	char *s;

	for (s = key,i = 0,hash = 0; *s && (i<MAX_KEY_LEN); s++, i++, hash *= 5) {
		hash += *s * coeff[i];
	}
	return hash;
}
 
/*
 * struct svalue_to_int: Converts an svalue into an integer index.
 */

INLINE int
svalue_to_int(v)
struct svalue *v;
{
   int i;

	debug(1,("mapping.c: struct svalue_to_int\n"));
	switch (v->type) {
	case T_NUMBER:
		i = v->u.number;
		if (i < 0)
			i = -i;
		return i;
	case T_STRING:
		return mapHashstr(v->u.string);
	case T_OBJECT:
		return mapHashstr(v->u.ob->name);
	case T_MAPPING :
		return (int)v->u.map;
	case T_POINTER :
		return (int)v->u.vec;
	default:
		return 0;  /* so much for our precious distribution */
	}
}

/*
 * node_find_in_mapping: Like find_for_insert(), but doesn't attempt
 * to add anything if a value is not found.  The returned pointer won't
 * necessarily have any meaningful value.
 */

INLINE struct node *
node_find_in_mapping(m, lv)
struct mapping *m;
struct svalue *lv;
{
	int i;
	struct node *elt;
 
	debug(1,("mapping.c: find_in_mapping\n"));
	if (!m->table) {
		return (struct node *)0;
	}
	i = svalue_to_int(lv) & (m->table_size - 1);
	for (elt = m->table[i]; elt; elt = elt->next) {
		if (sameval(&elt->values[0], lv))
			return elt;
	}
	return (struct node *)0;
}

/*
   mapping_delete: delete an element from the mapping
*/

void mapping_delete(m, lv)
struct mapping *m;
struct svalue *lv;
{
	struct node **n, **prev, *elt;
	int i;

	if (!(n = m->table))
		return;

	/*
	   zero m->elt to prevent all hell from breaking loose if delete is
	   called while iterating using each().
	*/
#ifdef EACH
	m->elt = (struct node *)0;
#endif
	i = svalue_to_int(lv) & (m->table_size - 1);
	prev = n + i;
	for (elt = n[i]; elt; elt = elt->next) {
		if (sameval(&elt->values[0], lv)) {
			*prev = elt->next;
			break;
		}
		prev = &elt->next;
	}
	if (!elt)
		return;
	debug(1024,("mapping delete: found element\n"));
	if (!n[i]) { /* caused bucket to become empty */
		m->filled--;
		debug(1024,("mapping delete: bucket empty, filled = \n", m->filled));
	}
	m->count--;
	total_mapping_nodes--;
	debug(1024,("mapping delete: count = \n", m->count));
	doFree(m, elt, NULL);
}
 
/*
 * find_for_insert: Tries to find an address at which an rvalue
 * can be inserted in a mapping.  This can also be used by the
 * microcode interpreter, to translate an expression <mapping>[index]
 * into an lvalue.
 */

struct svalue *
find_for_insert(m, lv, doTheFree)
struct mapping *m;
struct svalue *lv;
int doTheFree;
{
	int oi, i;
	struct node *n, *newnode;
 
	oi = svalue_to_int(lv);
	i = oi & (m->table_size - 1);
	debug(128,("mapping.c: hashed to %d\n", i));
	for (n = m->table[i]; n; n = n->next) {
		if (sameval(lv, &n->values[0])) {
			/* normally, the f_assign would free the old value */
			debug(128,("mapping.c: found %x\n", &n->values[0]));
			if (doTheFree) {
				free_svalue(&n->values[1]);
			}
			return &n->values[1];
		}
	}
	debug(128,("mapping.c: didn't find %x\n", lv));
	if (m->count > MAX_MAPPING_SIZE) {
		debug(128,("mapping.c: too full\n", lv));
		return (struct svalue *)0;
	}
	add_array_size (&m->stats, 2);
	total_mapping_size += sizeof(struct node);
	debug(128,("mapping.c: allocated a node\n"));
	newnode = (struct node *) DXALLOC(sizeof(struct node), 76,
		"find_for_insert");
	assign_svalue_no_free(&newnode->values[0], lv);
	newnode->values[1].type = T_NUMBER;
	newnode->values[1].subtype = T_NULLVALUE;
	newnode->values[1].u.number = 0;
	newnode->hashval = oi;
	/* insert at head of bucket */
	n = m->table[i];
	if (!n) {       /* bucket was empty */
		m->filled++;
	}
	m->table[i] = newnode;
	newnode->next = n;
	m->count++; /* increment count of number of nodes in mapping */
	total_mapping_nodes++;
	if (m->filled == m->do_split) {
		growMap(m);  /* double the size of the hash table */
	}
	return &newnode->values[1];
}
 
/*
 * load_mapping_from_aggregate: Create a new mapping, loading from an
 * array of svalues. Format of data: LHS RHS LHS2 RHS2... (uses hash table)
 */

struct mapping *
load_mapping_from_aggregate(sp, n)
struct svalue *sp;
int n;
{
	int i;
	struct mapping *m;
	struct svalue *v;
 
	debug(128,("mapping.c: load_mapping_from_aggregate begin, size = %d\n", n));
	m = allocate_mapping(n / 2);
	for (i = 0; i < n; i += 2) {
		v = find_for_insert(m, sp + i, 1);
		if (v) {
			assign_svalue_no_free(v, sp + i + 1);
		} else {
			mapping_too_large();
			break;
		}
	}
	debug(128,("mapping.c: load_mapping_from_aggregate end\n"));
	return m;
}

/* is ok */

struct svalue *
find_in_mapping(m, lv)
struct mapping *m;
struct svalue *lv;
{
   struct node *n;
 
	n = node_find_in_mapping(m, lv);
	if (!n)
		return &const0u;
	return &n->values[1];
}

void
absorb_mapping(m1, m2)
struct mapping *m1, *m2;
{
	if (m2->count) {
		mapTraverse(m2, (int (*)())doInsert, m1);
	}
}

/*
   add_mapping: returns a new mapping that contains everything
   in two old mappings.  (uses hash table)
*/

struct mapping *
add_mapping(m1, m2)
struct mapping *m1, *m2;
{
	struct mapping *newmap;
 
	debug(128,("mapping.c: add_mapping begin: %x, %x\n", m1, m2));
	newmap = allocate_mapping(m1->count);
	if (m1->count)
		mapTraverse(m1, (int (*)())doInsert, newmap);
	if (m2->count)
		mapTraverse(m2, (int (*)())doInsert, newmap);
	debug(128,("mapping.c: add_mapping end\n"));
	return newmap;
}

/*
  doTransform: called by mapTraverse (via map_mapping)
*/

int doTransform(m, elt, info)
struct mapping *m;
struct node *elt;
finfo_t *info;
{
	struct svalue *ret;

	push_svalue(&elt->values[1]);
	if (info->extra) {
		push_svalue(info->extra);
		ret = apply(info->func, info->obj, 2);
	}
	ret = apply(info->func, info->obj, 1);
	if (ret) {
		assign_svalue(&elt->values[1], ret);
		return 0;
	}
	/* might as well stop traversing if func doesn't exist in object */
	return 1;
}
 
/*
   map_mapping: A lot of the efuns that work on arrays, such as
   filter_array(), should also work on mappings.
*/

struct mapping *
map_mapping(m, func, ob, extra)
struct mapping *m;
object_t *ob;
char *func;
struct svalue *extra;
{
	finfo_t info;
 
	debug(1,("mapping.c: map_mapping\n"));
	info.func = func;
	info.obj = ob;
	info.extra = extra;
	return mapTraverse(copyMapping(m), (int (*)())doTransform, &info);
}

int doCompose(m, elt, info)
struct mapping *m;
struct node *elt;
minfo_t *info;
{
	struct node *node;
	struct svalue *value;

	if ((node = node_find_in_mapping(info->map, &elt->values[1]))) {
		value = find_for_insert(info->newmap, &elt->values[1], 1);
		if (value) {
			assign_svalue_no_free(value, &node->values[1]);
		} else {
			mapping_too_large();
			return 1;
		}
	}
	return 0;
}

/* compose_mapping */

struct mapping *
compose_mapping(m1, m2)
struct mapping *m1, *m2;
{
	minfo_t info;

	debug(1,("mapping.c: compose_mapping\n"));
	info.map = m2;
	info.newmap = allocate_mapping(0);
	mapTraverse(m1, (int (*)())doCompose, &info);
	return info.newmap;
}

/*
  doGetKey: called by mapTraverse from mapping_indices
*/

int doGetOne(m, elt, info)
struct mapping *m;
struct node *elt;
vinfo_t *info;
{
	if (info->pos == info->size)
		return 1;
	assign_svalue_no_free(&info->v->item[info->pos++], &elt->values[info->w]);
	return 0;
}

/* mapping_indices */

struct vector *
mapping_indices(m)
struct mapping *m;
{
	int size;
	vinfo_t info;

	size = m->count;
	if (size > MAX_ARRAY_SIZE)
		size = MAX_ARRAY_SIZE;
	debug(128,("mapping_indices: size = %d\n",size));
	info.v = allocate_array(size); /* assigns ref count equal to 1 */
	info.pos = 0;
	info.size = size;
	info.w = 0;
	mapTraverse(m, (int (*)())doGetOne, &info);
	return info.v;
}

/* mapping_values */

struct vector *
mapping_values(m)
struct mapping *m;
{
	int size;
	vinfo_t info;

	size = m->count;
	if (size > MAX_ARRAY_SIZE)
		size = MAX_ARRAY_SIZE;
	debug(64,("size = %d\n",size));
	info.v = allocate_array(size); /* assigns ref count equal to 1 */
	info.pos = 0;
	info.size = size;
	info.w = 1;
	mapTraverse(m, (int (*)())doGetOne, &info);
	return info.v;
}

#ifdef EACH
/* mapping_each */

INLINE struct vector *
mapping_each(m)
struct mapping *m;
{
	int j;
	struct vector *v;

	if (!m->count) { /* map is empty */
		return null_array();
	}
	/*
	  If each() being called by a different object than previous object,
	  then reset so that each begins again at top of map.  This is necessary
	  so that things aren't left in a bad state of an object errors out
	  in the middle of traversing a map (or just doesn't traverse to the end).
	*/
	if (current_object != m->eachObj) {
		m->eachObj = current_object;
		m->bucket = 0;
		m->elt = (struct node *)0;
	}
	if (!m->elt) { /* find next occupied bucket in hash table */
		int found = 0;

		for (j = m->bucket; j < (int)m->table_size; j++) {
			if (m->table[j]) {
				m->bucket = j + 1;
				m->elt = m->table[j];
				found = 1;
				break;
			}
		}
		if (!found) {
			m->bucket = 0;
			m->elt = (struct node *)0;
			return null_array();  /* have reached the end */
		}
	}
	v = allocate_array(2);
	assign_svalue_no_free(&v->item[0], &m->elt->values[0]);
	assign_svalue_no_free(&v->item[1], &m->elt->values[1]);
	m->elt = m->elt->next;
	return v;
}
#endif

int doSummation(m, elt, size)
struct mapping *m;
struct node *elt;
int *size;
{
	/* don't need to account for : and , since the other "size"
	   computing functions account for the following delimeter */
	*size += svalue_save_size(&elt->values[0]) +
		svalue_save_size(&elt->values[1]); 
	return 0;
}

int mapping_save_size(m)
struct mapping *m;
{
	int size;

	size = 0;
	mapTraverse(m, (int (*)())doSummation, &size);
	return size;
}


/*
 * Encode a mapping of elements into a contiguous string.
 */

int doEncode(m, elt, buf)
struct mapping *m;
struct node *elt;
char **buf;
{
	save_svalue(&elt->values[0], buf);
	bufcat(buf, ":");
	save_svalue(&elt->values[1], buf);
	bufcat(buf, ",");
	return 0;
}

char *save_mapping(m)
struct mapping *m;
{
	char *buf, *p;

	/* 5 == strlen("([])") */
	buf = DXALLOC(mapping_save_size(m) + 5, 77, "save_mapping");

	p = buf;
	bufcat(&p, "([");
	mapTraverse(m, (int (*)())doEncode, &p);
	bufcat(&p, "])");
	return buf;
}
