/* 92/04/18 - cleaned up in accordance with ./src/style.guidelines */

#include "std.h"
#include "config.h"
#include "lpc_incl.h"
#include "debug.h" /* added by Truilkan */
#include "md.h"

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

/*
  growMap: based on hash.c:hsplit() from Larry Wall's Perl.

  growMap doubles the size of the hash table.  It is called when FILL_PERCENT
  of the buckets in the hash table contain values.  This routine is
  efficient since the entries in the table don't need to be rehashed (even
  though the entries are redistributed across the both halves of the hash
  table).
*/

INLINE int growMap P1(mapping_t *, m)
{
	int oldsize = m->table_size + 1;
	int newsize = oldsize << 1;
	int i;
	mapping_node_t **a, **b, **eltp, *elt;

	if (newsize > MAX_TABLE_SIZE) 
		return 0;
	/* resize the hash table to be twice the old size */
	m->table = a = RESIZE(m->table, newsize, mapping_node_t *, TAG_MAP_TBL, "growMap");
	if (!a) {
		/*
		  We couldn't grow the hash table.  Rather than die, we just
		  accept the performance hit resulting from having an overfull table.
		*/
		m->unfilled = m->table_size;
		return 0;
	}
	/* hash table doubles in size -- keep track of the memory used */
	total_mapping_size += sizeof(mapping_node_t *) * oldsize;
	debug(1024,("mapping.c: growMap ptr = %x, size = %d\n", m, newsize));
	m->unfilled = oldsize * (unsigned)FILL_PERCENT / (unsigned)100;
	m->table_size = newsize - 1;
	/* zero out the new storage area (2nd half of table) */
	memset(a += oldsize, 0, oldsize * sizeof(mapping_node_t *));
	i = oldsize;
	while (a--, i--){
	    if ((elt = *a)) {
		eltp = a, b = a + oldsize;
		do {
		    if (elt->hashval & oldsize){
			*eltp = elt->next;
			if (!(elt->next = *b)) m->unfilled--;
			*b = elt;   
			elt = *eltp;
		    }
		    else elt = *(eltp = &elt->next);
		} while (elt);
		if (!*a) m->unfilled++;
	    }
	}
	return 1;
}

/*
  mapTraverse: iterate over the mapping, calling function 'func(elt, extra)'
  for each element 'elt'.  This is an attempt to encapsulate some of the
  specifics of the particular data structure being used so that it won't be
  so difficult to change the data structure if the need arises.
  -- Truilkan 92/07/19
*/

INLINE mapping_t *
mapTraverse (m, func, extra)
mapping_t *m;
int (*func) PROT((mapping_t *, mapping_node_t *, void *));
void *extra;
{
	mapping_node_t *elt, *nelt;
	int j = (int) m->table_size;
	
	debug(128,("mapTraverse %x\n", m));
	do {
	    for (elt = m->table[j]; elt; elt = nelt){
		nelt = elt->next;
		if ((*func)(m, elt, extra)) return m;
	    }
	} while (j--);
	return m;
}

/* free_mapping */
 
INLINE void
dealloc_mapping P1(mapping_t *, m)
{
	debug(1024,("mapping.c: actual free of %x\n", m));
	num_mappings--;
	{
	    int j = m->table_size, c = m->count;
	    mapping_node_t *elt, *nelt, **a = m->table;
	    
	    total_mapping_size -= (sizeof(mapping_t) +
				   sizeof(mapping_node_t *) * (j+1) +
				   sizeof(mapping_node_t) * c);
	    total_mapping_nodes -= c;
#ifdef PACKAGE_MUDLIB_STATS
	    add_array_size (&m->stats, - (c << 1));
#endif

	    do {
		for (elt = a[j]; elt; elt = nelt){
		    nelt = elt->next;
		    free_svalue(elt->values + 1, "free_mapping");
		    free_svalue(elt->values, "free_mapping");
		    free_node(elt);

		}
	    } while (j--);
	

	    debug(2048, ("in free_mapping: before table\n"));
	    FREE((char *)a);
	}

	debug(2048, ("in free_mapping: after table\n"));
	FREE((char *) m);
	debug(2048, ("in free_mapping: after m\n"));
	debug(64,("mapping.c: free_mapping end\n"));
}

INLINE void
free_mapping P1(mapping_t *, m)
{
	debug(64,("mapping.c: free_mapping begin, ptr = %x\n", m));
	/* some other object is still referencing this mapping */
	if (--m->ref > 0)
		return;
	dealloc_mapping(m);
}

static mapping_node_t *free_nodes = 0;
mapping_node_block_t *mapping_node_blocks = 0;

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_mapping_node_blocks() {
    mapping_node_block_t *mnb = mapping_node_blocks;

    while (mnb) {
	DO_MARK(mnb, TAG_MAP_NODE_BLOCK);
	mnb = mnb->next;
    }
}
#endif

mapping_node_t *new_map_node() {
    mapping_node_block_t *mnb;
    mapping_node_t *ret;
    int i;

    if ((ret = free_nodes)) {
	free_nodes = ret->next;
    } else {
	mnb = ALLOCATE(mapping_node_block_t, TAG_MAP_NODE_BLOCK, "new_map_node");
	mnb->next = mapping_node_blocks;
	mapping_node_blocks = mnb;
	mnb->nodes[MNB_SIZE - 1].next = 0;
	for (i = MNB_SIZE - 1; i--; )
	    mnb->nodes[i].next = &mnb->nodes[i+1];
	ret = &mnb->nodes[0];
	free_nodes = &mnb->nodes[1];
    }
    return ret;
} 

void free_node P1(mapping_node_t *, mn) {
    mn->next = free_nodes;
    free_nodes = mn;
}

/* allocate_mapping(int n)
   
   call with n == 0 if you will be doing many deletions from the map.
   call with n == "approx. # of insertions you plan to do" if you won't be
   doing many deletions from the map.
*/
 
INLINE mapping_t *
allocate_mapping P1(int, n)
{
	mapping_t *newmap;
	mapping_node_t **a;

	if (n > MAX_MAPPING_SIZE) n = MAX_MAPPING_SIZE;
	newmap = ALLOCATE(mapping_t, TAG_MAPPING, "allocate_mapping: 1");
	debug(1024,("mapping.c: allocate_mapping begin, newmap = %x\n", newmap));
	if (newmap == NULL) 
	    error("Allocate_mapping - out of memory.\n");

	if (n > MAP_HASH_TABLE_SIZE){
	    n |= n >> 1;
	    n |= n >> 2;
	    n |= n >> 4;
	    if (n & 0xff00) n |= n >> 8;
	    newmap->table_size = n++;
	}
	else newmap->table_size = (n = MAP_HASH_TABLE_SIZE) - 1;
	/* The size is actually 1 higher */
	newmap->unfilled = n * (unsigned)FILL_PERCENT /(unsigned)100;
	a = newmap->table = 
	    (mapping_node_t **)DXALLOC(n *= sizeof(mapping_node_t *),
				    TAG_MAP_TBL, "allocate_mapping: 3");
	if (!a)
	    error("Allocate_mapping 2 - out of memory.\n");
	/* zero out the hash table */
	memset(a, 0, n);
	total_mapping_size += sizeof(mapping_t) + n;
	newmap->ref = 1;
	newmap->count = 0;
#ifdef EACH
	newmap->eachObj = (object_t *)0;
	newmap->bucket = 0;
	newmap->elt = (mapping_node_t *)0;      /* must start at 0;see mapping_each */
#endif
#ifdef PACKAGE_MUDLIB_STATS
	if (current_object) {
	  assign_stats (&newmap->stats, current_object);
	  add_array_size (&newmap->stats, n << 1);
	} else {
	  null_stats (&newmap->stats);
	}
#endif
	num_mappings++;
	debug(64,("mapping.c: allocate_mapping end\n"));
	return newmap;
}

/*
  copyMapping: make a copy of a mapping
*/

INLINE mapping_t *
copyMapping P1(mapping_t *,m)
{
    mapping_t *newmap;
    int k = m->table_size;
    mapping_node_t *elt, *nelt, **a, **b = m->table, **c;

    newmap = ALLOCATE(mapping_t, TAG_MAPPING, "copy_mapping: 1");
    if (newmap == NULL) error("copyMapping - out of memory.\n");
    newmap->table_size = k++;
    newmap->unfilled = m->unfilled;
    newmap->ref = 1;
    c = newmap->table = CALLOCATE(k, mapping_node_t *, TAG_MAP_TBL, "copy_mapping: 2");
    if (!c){
	FREE((char *) newmap);
	error("copyMapping 2 - out of memory.\n");
    }
    total_mapping_nodes += (newmap->count = m->count);
    memset(c, 0, k * sizeof(mapping_node_t *));
    total_mapping_size += (sizeof(mapping_t) +
			   sizeof(mapping_node_t *) * k +
			   sizeof(mapping_node_t) * m->count);

#ifdef EACH
    newmap->eachObj = (object_t *)0;
    newmap->bucket = 0;
    newmap->elt = (mapping_node_t *)0;      /* must start at 0;see mapping_each */
#endif
#ifdef PACKAGE_MUDLIB_STATS
    if (current_object) {
	assign_stats (&newmap->stats, current_object);
	add_array_size (&newmap->stats, m->count << 1);
    }
    else null_stats (&newmap->stats);
#endif
    num_mappings++;
    while (k--){
	if ((elt = b[k])) {
	    a = c + k;
	    do {
		nelt = new_map_node();

		assign_svalue_no_free(nelt->values, elt->values);
		assign_svalue_no_free(nelt->values + 1, elt->values + 1);
		nelt->hashval = elt->hashval;
		nelt->next = *a;
		*a = nelt;
	    } while ((elt = elt->next));
	}
    }
    return newmap;
}

/*
   mapHashstr: based on code from hstore in Larry Wall's Perl package (hash.c)

   I use this hash function instead of hash.c:hashstr() because the scheme
   we use for growing the hash table requires the table size to be a power
   of 2 and I don't that the other hash function performs well for non-
   prime moduli.
*/

/* Beek - whashstr supposedly needs even power of 2 sizes; will it work
 * well for odd powers of 2?  Can it cope with the table size changing?
 * anyway, I think we can do better than this hash function ...
 */
INLINE int
mapHashstr P1(register char *, key)
{
	int hash = 0, i = 0;

	while (*key && (i < MAX_KEY_LEN)){
	    hash += (*key++) * coeff[i++];
	    hash *= 5;
	}
	return hash;
}


INLINE int
restore_hash_string P3(char **, val, int *, a, svalue_t *, sv)
{
    int hash = 0, i = 0;
    register char *cp = *val;
    char c, *start = cp;
    int len;

    while ((c = *cp++) != '"'){
	switch(c){
#ifndef MSDOS
	    case '\r':
#else
	    case 30:
#endif
	    {
		*(cp-1) = '\n';
		if (i < MAX_KEY_LEN){
		    hash += '\n' * coeff[i++];
		    hash *= 5;
		}
		break;
	    }

	    case '\\':
	    {
                char *new = cp - 1;

                if ((c = *new++ = *cp++)) {
		    if (i < MAX_KEY_LEN){
			hash += c * coeff[i++];
			hash *= 5;
		    }
                    while ((c = *cp++) != '"'){
                        if (c == '\\'){
                            if (!(c = *new++ = *cp++)) return ROB_STRING_ERROR;
			}
                        else {
#ifndef MSDOS
                            if (c == '\r')
#else
                            if (c == 30)
#endif
                                c = *new++ = '\n';
                            else *new++ = c;
			}
			if (i < MAX_KEY_LEN) {
			    hash += c * coeff[i++];
			    hash *= 5;
			}
		    }
                    if (!c) return ROB_STRING_ERROR;
                    *new = '\0';
                    *val = cp;
                    sv->u.string = new_string(len = (new - start),
					      "restore_string");
                    strcpy(sv->u.string, start);
		    sv->type = T_STRING;
		    sv->subtype = STRING_MALLOC;
		    *a = hash;
                    return 0;
		}
                else return ROB_STRING_ERROR;
	    }

	    case '\0':
	    {
		return ROB_STRING_ERROR;
	    }

	    default:
	    {
		if (i < MAX_KEY_LEN){
		    hash += c * coeff[i++];
		    hash *= 5;
		}
	    }
	}
    }
    *val = cp;
    *--cp = '\0';
    len = cp - start;
    sv->u.string = new_string(len, "restore_string");
    strcpy(sv->u.string, start);
    *a = hash;
    sv->type = T_STRING;
    sv->subtype = STRING_MALLOC;
    return 0;
}
    
/*
 * svalue_t_to_int: Converts an svalue into an integer index.
 */

INLINE int
svalue_to_int P1(svalue_t *, v)
{
	debug(1,("mapping.c: svalue_t_to_int\n"));
	switch (v->type) {
	case T_NUMBER:
	    {
		int i;
		return ((i = v->u.number) < 0) ? -i : i;
	    }
	case T_STRING:
	    return mapHashstr(v->u.string);
	case T_OBJECT:
	    return mapHashstr(v->u.ob->name);
	case T_MAPPING:
	    return (POINTER_INT)v->u.map;
	case T_ARRAY:
	case T_CLASS:
	    return (POINTER_INT)v->u.arr;
	case T_FUNCTION:
	    return (POINTER_INT)v->u.fp;
	case T_REAL:
	    return (POINTER_INT)v->u.real;
	case T_BUFFER:
	    return (POINTER_INT)v->u.buf;
	default:
	    return 0;  /* so much for our precious distribution */
	}
}

/*
 * node_find_in_mapping: Like find_for_insert(), but doesn't attempt
 * to add anything if a value is not found.  The returned pointer won't
 * necessarily have any meaningful value.
 */

INLINE mapping_node_t *
node_find_in_mapping P2(mapping_t *, m, svalue_t *, lv)
{
	int i;
	mapping_node_t *elt, **a = m->table;
 
	debug(1,("mapping.c: find_in_mapping\n"));
	if (!m->table) {
		return (mapping_node_t *)0;
	}
	i = svalue_to_int(lv) & m->table_size;
	for (elt = a[i]; elt; elt = elt->next) {
		if (sameval(elt->values, lv))
			return elt;
	}
	return (mapping_node_t *)0;
}

/*
   mapping_delete: delete an element from the mapping
*/

INLINE void mapping_delete P2(mapping_t *,m, svalue_t *,lv)
{
	int i = svalue_to_int(lv) & m->table_size;
	mapping_node_t **prev = m->table + i, *elt;
	/*
	   zero m->elt to prevent all hell from breaking loose if delete is
	   called while iterating using each().
	*/
#ifdef EACH
	m->elt = (mapping_node_t *)0;
#endif
	if ((elt = *prev)) {
	    do {
		if (sameval(elt->values, lv)){
		    if (!(*prev = elt->next) && !m->table[i]){
			m->unfilled++;
			debug(1024,("mapping delete: bucket empty, unfilled = \n",
				    m->unfilled));
		    }
		    m->count--;
		    total_mapping_nodes--;
		    total_mapping_size -= sizeof(mapping_node_t);
		    debug(1024,("mapping delete: count = \n", m->count));
		    free_svalue(elt->values + 1, "mapping_delete");
		    free_svalue(elt->values, "mapping_delete");
		    free_node(elt);
		    return;

		}
		prev = &(elt->next);
	    } while ((elt = elt->next));

	} 
}
 
/*
 * find_for_insert: Tries to find an address at which an rvalue
 * can be inserted in a mapping.  This can also be used by the
 * microcode interpreter, to translate an expression <mapping>[index]
 * into an lvalue.
 */

INLINE svalue_t *
find_for_insert P3(mapping_t *, m, svalue_t *, lv, int, doTheFree)
{
	int oi = svalue_to_int(lv);
	unsigned short i = oi & m->table_size;
	mapping_node_t *n, *newnode, **a = m->table + i;
 
	debug(128,("mapping.c: hashed to %d\n", i));
	if ((n = *a)) {
	    do {
		if (sameval(lv, n->values)) {
		    /* normally, the f_assign would free the old value */
		    debug(128,("mapping.c: found %x\n", n->values));
		    if (doTheFree) free_svalue(n->values + 1, "find_for_insert");
		    return n->values + 1;
		}
	    } while ((n = n->next));
	    debug(128,("mapping.c: didn't find %x\n", lv));
	    n = *a;
	}
	else if (!(--m->unfilled)){
	    int size = m->table_size + 1;

	    if (growMap(m)){ 
		if (oi & size) i |= size;
		n = *(a = m->table + i);
	    } else {
		error("Out of memory\n");
	    }
	}

	if (++m->count > MAX_MAPPING_SIZE) {
		m->count--;
		debug(128,("mapping.c: too full\n", lv));
		mapping_too_large();
	}
#ifdef PACKAGE_MUDLIB_STATS
	add_array_size (&m->stats, 2);
#endif
	total_mapping_size += sizeof(mapping_node_t);
	debug(128,("mapping.c: allocated a node\n"));
	newnode = new_map_node();
	assign_svalue_no_free(newnode->values, lv);
	newnode->hashval = oi;
	/* insert at head of bucket */
	(*a = newnode)->next = n;
	(lv = newnode->values + 1)->type = T_NUMBER;
	lv->subtype = T_NULLVALUE;
	lv->u.number = 0;
	total_mapping_nodes++;
	return lv;
}
 
#ifdef F_UNIQUE_MAPPING

typedef struct unique_node_s {
    svalue_t key;
    int count;
    unsigned short hashval;
    struct unique_node_s *next;
    int *indices;
} unique_node_t;

typedef struct unique_m_list_s {
    unique_node_t **utable;
    struct unique_m_list_s *next;
    unsigned short mask;
} unique_m_list_t;

static unique_m_list_t *g_u_m_list = 0;

void unique_mapping_error_handler PROT((void))
{
    unique_m_list_t *nlist = g_u_m_list;
    unique_node_t **table = nlist->utable;
    unique_node_t *uptr, *nptr;
    int mask = nlist->mask;

    g_u_m_list = g_u_m_list->next;

    do {
        if ((uptr = table[mask])) {
            do {
                nptr = uptr->next;
		free_svalue(&uptr->key, "unique_mapping_error_handler");
                FREE((char *) uptr->indices);
                FREE((char *) uptr);
            } while ((uptr = nptr));
        }
    } while (mask--);
    FREE((char *) table);
    FREE((char *) nlist);
}

void f_unique_mapping PROT((void))
{
    unique_m_list_t *nlist;
    funptr_t *fp;
    object_t *ob = 0;
    char *func;
    svalue_t *arg = sp - st_num_arg + 1, *extra, *sv;
    unique_node_t **table, *uptr, *nptr;
    array_t *v = arg->u.arr, *ret;
    unsigned short oi, i, numkeys = 0, mask, size;
    unsigned short num_arg = st_num_arg, numex = 0;
    unsigned short nmask;
    mapping_t *m;
    mapping_node_t **mtable, *elt;
    int *ind, j;

    if ((arg+1)->type == T_FUNCTION){
        fp = (arg+1)->u.fp;
        if (num_arg > 2) extra = arg + 2, numex = num_arg - 2;
    } else {
        func = (arg+1)->u.string;
        if (num_arg < 3) ob = current_object;
        else {
            if ((arg +2)->type == T_OBJECT) ob = (arg+2)->u.ob;
            else if ((arg+2)->type == T_STRING){
                if ((ob = find_object(arg[2].u.string)) && !object_visible(ob)) ob = 0;
            }
            if (!ob) bad_argument(arg+2, T_STRING | T_OBJECT, 3, F_UNIQUE_MAPPING);
            if (num_arg > 3) extra = arg + 3, numex = num_arg - 3;
        }
    }

    size = v->size;
    if (!size){
        pop_n_elems(num_arg - 1);
        free_array(v);
        sp->type = T_MAPPING;
        sp->u.map = allocate_mapping(0);
        return;
    }

    if (size > MAP_HASH_TABLE_SIZE){
        size |= size >> 1;
        size |= size >> 2;
        size |= size >> 4;
        if (size & 0xff00) size |= size >> 8;
        mask = size++;
    } else mask = (size = MAP_HASH_TABLE_SIZE) - 1;

    table = (unique_node_t **) DXALLOC(size *= sizeof(unique_node_t *),
                                            100, "f_unique_mapping:1");
    if (!table) error("Unique_mapping - Out of memory.\n");
    memset(table, 0, size);

    nlist = ALLOCATE(unique_m_list_t, 101, "f_unique_mapping:2");
    nlist->next = g_u_m_list;
    nlist->utable = table;
    nlist->mask = mask;
    g_u_m_list = nlist;

    (++sp)->type = T_ERROR_HANDLER;
    sp->u.error_handler = unique_mapping_error_handler;

    size = v->size;
    while (size--){
        push_svalue(v->item + size);
        if (numex) push_some_svalues(extra, numex);
        sv = ob ? apply(func, ob, 1 + numex, ORIGIN_EFUN) : call_function_pointer(fp, 1 + numex);
        i = (oi = svalue_to_int(sv)) & mask;
        if ((uptr = table[i])) {
            do {
                if (sameval(&uptr->key, sv)){
                    ind = uptr->indices = RESIZE(uptr->indices, uptr->count+1,
						 int, 102, "f_unique_mapping:3");
                    ind[uptr->count++] = size;
                    break;
                }
            } while ((uptr = uptr->next));
        }
        if (!uptr){
            uptr = ALLOCATE(unique_node_t, 103, "f_unique_mapping:4");
            assign_svalue_no_free(&uptr->key, sv);
            uptr->count = 1;
            uptr->indices = ALLOCATE(int, 104, "f_unique_mapping:5");
            uptr->indices[0] = size;
            uptr->next = table[i];
            uptr->hashval = oi;
            table[i] = uptr;
            numkeys++;
        }
    }

    m = allocate_mapping(nmask = numkeys << 1);
    mtable = m->table;
    numkeys = 0;

    if (nmask > MAP_HASH_TABLE_SIZE){
        nmask |= nmask >> 1;
        nmask |= nmask >> 2;
        nmask |= nmask >> 4;
        if (size & 0xff00) nmask |= nmask >> 8;
    } else nmask = MAP_HASH_TABLE_SIZE - 1;
    j = mask;
    sv = v->item;

    do {
        if ((uptr = table[j])) {
            do {
                nptr = uptr->next;
                i = (oi = uptr->hashval) & nmask;
                if (!mtable[i] && !(--m->unfilled)){
                    if (growMap(m)){
                        mtable = m->table;
                        nmask <<= 1;
                        nmask--;
                    } else {
			do {
			    do {
				nptr = uptr->next;
				free_svalue(&uptr->key, "f_unique_mapping");
				FREE((char *) uptr->indices);
				FREE((char *) uptr);
			    } while ((uptr = nptr));
			    uptr = table[--j];
			} while (j >= 0);
#ifdef PACKAGE_MUDLIB_STATS
                        add_array_size(&m->stats, numkeys << 1);
#endif
                        total_mapping_size += sizeof(mapping_node_t) * (m->count = numkeys);
                        total_mapping_nodes += numkeys;
                        free_mapping(m);
                        error("Out of memory\n");
                    }
                }

                elt = ALLOCATE(mapping_node_t, 105,"f_unique_mapping:6");
                elt->hashval = oi;
                *elt->values = uptr->key;
                (elt->values + 1)->type = T_ARRAY;
                ret = (elt->values + 1)->u.arr = allocate_empty_array(size = uptr->count);
                ind = uptr->indices;
                while (size--){
                    assign_svalue_no_free(ret->item + size, sv + ind[size]);
                }
                elt->next = mtable[i];
                mtable[i] = elt;
                FREE((char *) ind);
                FREE((char *) uptr);
                numkeys++;
            } while ((uptr = nptr));
        }
    } while (j--);

#ifdef PACKAGE_MUDLIB_STATS
    add_array_size(&m->stats, numkeys << 1);
#endif
    total_mapping_size += sizeof(mapping_node_t) * (m->count = numkeys);
    total_mapping_nodes += numkeys;
    FREE((char *) table);
    g_u_m_list = g_u_m_list->next;
    FREE((char *) nlist);
    sp--;
    pop_n_elems(num_arg - 1);
    free_array(v);
    sp->type = T_MAPPING;
    sp->u.map = m;
}
#endif /* End of unique_mapping */

/*
 * load_mapping_from_aggregate: Create a new mapping, loading from an
 * array of svalues. Format of data: LHS RHS LHS2 RHS2... (uses hash table)
 */

INLINE mapping_t *
load_mapping_from_aggregate P2(svalue_t *,sp, int, n)
{
	mapping_t *m;
	int mask, i, oi, count = 0;
	mapping_node_t **a, *elt, *elt2; 
 
	debug(128,("mapping.c: load_mapping_from_aggregate begin, size = %d\n", n));
	m = allocate_mapping(n >> 1);
	if (!n) return m;
	mask = m->table_size;
	a = m->table;
	do {
	    i = (oi = svalue_to_int(++sp)) & mask;
	    if ((elt2 = elt = a[i])) {
		do {
		    if (sameval(sp, elt->values)) {
			free_svalue(sp++, "load_mapping_from_aggregate: duplicate key");
			free_svalue(elt->values+1, "load_mapping_from_aggregate");
			*(elt->values+1) = *sp;
			break;
		    }
		} while ((elt = elt->next));
		if (elt) continue;
	    }
	    else if (!(--m->unfilled)){
		if (growMap(m)){
		    a = m->table;
		    if (oi & ++mask) elt2 = a[i |= mask];
		    mask <<= 1;
		    mask--;
		} else{
#ifdef PACKAGE_MUDLIB_STATS
		    add_array_size(&m->stats, count << 1);
#endif
		    total_mapping_size += sizeof(mapping_node_t) * (m->count = count);
		    total_mapping_nodes += count;
		    free_mapping(m);
		    error("Out of memory\n");
		}
	    }

	    if (++count > MAX_MAPPING_SIZE){
#ifdef PACKAGE_MUDLIB_STATS
		add_array_size(&m->stats, (--count) << 1);
#endif
		total_mapping_size += sizeof(mapping_node_t) * (m->count = count);
		total_mapping_nodes += count;
		
		free_mapping(m);
		mapping_too_large();
	    }

	    elt = new_map_node();
	    *elt->values = *sp++;
	    *(elt->values + 1) = *sp;
	    elt->hashval = oi;
	    (a[i] = elt)->next = elt2;
	} while (n -= 2);
#ifdef PACKAGE_MUDLIB_STATS
	add_array_size(&m->stats, count << 1);
#endif
	total_mapping_size += sizeof(mapping_node_t) * (m->count = count);
	total_mapping_nodes += count;
	debug(128,("mapping.c: load_mapping_from_aggregate end\n"));
	return m;
}

/* is ok */

INLINE svalue_t *
find_in_mapping P2(mapping_t *, m, svalue_t *,lv)
{
	int i = svalue_to_int(lv) & m->table_size;
	mapping_node_t *n = m->table[i];

	while (n){
	    if (sameval(n->values, lv)) return n->values + 1;
	    n = n->next;
	}

	return &const0u;
}


/* 
    add_to_mapping: adds mapping m2 to m1 
*/

INLINE static void
add_to_mapping P3(mapping_t *,m1, mapping_t *,m2, int, free_flag)
{
    int mask = m1->table_size, j = m2->table_size;
    int count = m1->count;
    int i, oi;
    mapping_node_t *elt1, *elt2, *newnode, *n;
    mapping_node_t **a1 = m1->table, **a2 = m2->table;
    svalue_t *sv;

    do {
	for (elt2 = a2[j]; elt2; elt2 = elt2->next){
	    i = (oi = elt2->hashval) & mask;
	    sv = elt2->values;
	    if ((n = elt1 = a1[i])) {
		do {
		    if (sameval(sv, elt1->values)){
			assign_svalue(elt1->values + 1, sv + 1);
			break; 
		    }
		} while ((elt1 = elt1->next));
		if (elt1) continue;
	    } else if (!(--m1->unfilled)){
		if (growMap(m1)){
		    a1 = m1->table;
		    if (oi & ++mask) n = a1[i |= mask];
		    mask <<= 1;
		    mask--;
		} else{
		    count -= m1->count;
#ifdef PACKAGE_MUDLIB_STATS
		    add_array_size(&m1->stats, count << 1);
#endif
		    total_mapping_size += count * sizeof(mapping_node_t);
		    total_mapping_nodes += count;
		    m1->count += count;
		    if (free_flag) free_mapping(m1);
		    error("Out of memory\n");
		}
	    }
	    if (++count > MAX_MAPPING_SIZE){
		if (count -= m1->count + 1){	
#ifdef PACKAGE_MUDLIB_STATS
		    add_array_size(&m1->stats, count << 1);
#endif
		    total_mapping_size += count * sizeof(mapping_node_t);
		    total_mapping_nodes += count;
		}
		m1->count += count;
		mapping_too_large();
	    }

	    newnode = new_map_node();
	    assign_svalue_no_free(newnode->values, elt2->values);
	    assign_svalue_no_free(newnode->values+1,elt2->values+1);
	    newnode->hashval = oi;
	    (a1[i] = newnode)->next = n;
	}
    } while (j--);

    if (count -= m1->count){
#ifdef PACKAGE_MUDLIB_STATS
	add_array_size(&m1->stats, count << 1);
#endif
	total_mapping_size += count * sizeof(mapping_node_t);
	total_mapping_nodes += count;
    }

    m1->count += count;
}

/* 
    unique_add_to_mapping : adds m2 to m1 but doesn't do anything
		            if they have common keys 
*/

INLINE static void
unique_add_to_mapping P3(mapping_t *,m1, mapping_t *,m2, int,free_flag)
{
    int mask = m1->table_size, j = m2->table_size;
    int count = m1->count;
    int i, oi;
    mapping_node_t *elt1, *elt2, *newnode, *n;
    mapping_node_t **a1 = m1->table, **a2 = m2->table;
    svalue_t *sv;

    do {
	for (elt2 = a2[j]; elt2; elt2 = elt2->next){
	    i = (oi = elt2->hashval) & mask;
	    sv = elt2->values;
	    if ((n = elt1 = a1[i])) {
		do {
		    if (sameval(sv, elt1->values)) break;
		} while ((elt1 = elt1->next));
		if (elt1) continue;
	    }
	    else if (!(--m1->unfilled)){
		if (growMap(m1)){
		    a1 = m1->table;
		    if (oi & ++mask) n = a1[i |= mask];
		    mask <<= 1;
		    mask--;
		} else{
		    ++m1->unfilled;
		    count -= m1->count;
#ifdef PACKAGE_MUDLIB_STATS
		    add_array_size(&m1->stats, count << 1);
#endif
		    total_mapping_size += count * sizeof(mapping_node_t);
		    total_mapping_nodes += count;
		    m1->count += count;
		    if (free_flag) free_mapping(m1);

		    error("Out of memory\n");
		}
	    }

	    if (++count > MAX_MAPPING_SIZE){
		if (count -= m1->count + 1){
#ifdef PACKAGE_MUDLIB_STATS
		    add_array_size(&m1->stats, count << 1);
#endif
		    total_mapping_size += count * sizeof(mapping_node_t);
		    total_mapping_nodes += count;
		}
		m1->count += count;
		mapping_too_large();
	    }

	    newnode = new_map_node();
	    assign_svalue_no_free(newnode->values, elt2->values);
	    assign_svalue_no_free(newnode->values+1,elt2->values+1);
	    newnode->hashval = oi;
	    (a1[i] = newnode)->next = n;
	}
    } while (j--);

    if (count -= m1->count){
#ifdef PACKAGE_MUDLIB_STATS
	add_array_size(&m1->stats, count << 1);
#endif
	total_mapping_size += count * sizeof(mapping_node_t);
	total_mapping_nodes += count;
    }

    m1->count += count;
}

INLINE void
absorb_mapping(m1, m2)
mapping_t *m1, *m2;
{
	if (m2->count) add_to_mapping(m1, m2, 0);
}

/*
   add_mapping: returns a new mapping that contains everything
   in two old mappings.  (uses hash table)
*/

INLINE mapping_t *
add_mapping P2(mapping_t *,m1, mapping_t *,m2)
{
	mapping_t *newmap;
 
	debug(128,("mapping.c: add_mapping begin: %x, %x\n", m1, m2));
	if (m1->count >= m2->count){
	    if (m2->count){
		add_to_mapping(newmap = copyMapping(m1), m2, 1);
		return newmap;
	    }
	    else return copyMapping(m1);
	}
	else if (m1->count){
	    unique_add_to_mapping(newmap = copyMapping(m2), m1, 1);
	    return newmap;
	}   
	else return copyMapping(m2);
	debug(128,("mapping.c: add_mapping end\n"));
}

/*
   map_mapping: A lot of the efuns that work on arrays, such as
   filter_array(), should also work on mappings.
*/

void
map_mapping P2(svalue_t *, arg, int, num_arg)
{
    mapping_t *m = arg->u.map;
    mapping_node_t **a, *elt;
    int j = m->table_size, numex = 0;
    svalue_t *ret, *extra;
    funptr_t *fp;
    char *func;
    object_t *ob = 0;

    m = copyMapping(m);
    (++sp)->type = T_MAPPING;
    sp->u.map = m;

    if ((arg + 1)->type == T_FUNCTION){
	fp = (arg+1)->u.fp;
	if (num_arg > 2) extra = arg+2, numex = num_arg - 2;
    }
    else {
	func = (arg + 1)->u.string;
	if (num_arg < 3) ob = current_object;
	else {
	    if ((arg+2)->type == T_OBJECT) ob = (arg+2)->u.ob;
	    else if ((arg+2)->type == T_STRING){
		if ((ob = find_object((arg+2)->u.string)) && !object_visible(ob))
		    ob = 0;
	    } 
	    if (!ob)
		error("Bad argument 3 to map_mapping\n");
	    if (num_arg > 3) extra = arg +3, numex = num_arg - 3;
	}
    }
    a = m->table;
    debug(1,("mapping.c: map_mapping\n"));
    do {
	for (elt = a[j]; elt ; elt = elt->next){
	    push_svalue(elt->values);
	    push_svalue(elt->values+1);
	    if (numex) push_some_svalues(extra, numex);
	    ret = ob ? apply(func, ob, 2+numex, ORIGIN_EFUN) 
		: call_function_pointer(fp, 2+numex);
	    if (ret) assign_svalue(elt->values+1, ret);
	    else break;
	}
    } while (j--);

    sp--;
    pop_n_elems(num_arg);
    (++sp)->type = T_MAPPING;	
    sp->u.map = m;
}

#ifdef F_FILTER
void
filter_mapping P2(svalue_t *, arg, int, num_arg)
{
    mapping_t *m = arg->u.map, *newmap;
    mapping_node_t **a, *elt;
    mapping_node_t **b, *newnode, *n;
    int j = m->table_size, numex = 0, count = 0, size;
    svalue_t *ret, *extra;
    funptr_t *fp;
    char *func;
    object_t *ob = 0;
    unsigned short tb_index;

    newmap = allocate_mapping(0);
    (++sp)->type = T_MAPPING;
    sp->u.map = newmap;
    b = newmap->table;
    size = newmap->table_size;

    if ((arg + 1)->type == T_FUNCTION){
	fp = (arg+1)->u.fp;
	if (num_arg > 2) extra = arg+2, numex = num_arg - 2;
    }
    else {
	func = (arg + 1)->u.string;
	if (num_arg < 3) ob = current_object;
	else {
	    if ((arg+2)->type == T_OBJECT) ob = (arg+2)->u.ob;
	    else if ((arg+2)->type == T_STRING){
		if ((ob = find_object((arg+2)->u.string)) && !object_visible(ob))
		    ob = 0;
	    } 
	    if (!ob)
		error("Bad argument 3 to filter_mapping\n");
	    if (num_arg > 3) extra = arg +3, numex = num_arg - 3;
	}
    }
    a = m->table;
    debug(1,("mapping.c: filter_mapping\n"));
    do {
	for (elt = a[j]; elt ; elt = elt->next){
	    push_svalue(elt->values);
	    push_svalue(elt->values+1);
	    if (numex) push_some_svalues(extra, numex);
	    ret = ob ? apply(func, ob, 2+numex, ORIGIN_EFUN) 
		: call_function_pointer(fp, 2+numex);
	    if (!ret) break;
	    else if (ret->type != T_NUMBER || ret->u.number){
		tb_index = elt->hashval & size;
		b = newmap->table + tb_index;
		if (!(n = *b) && !(--newmap->unfilled)){
		    size++;
		    if (growMap(newmap)){
			if (elt->hashval & size) tb_index |= size;
			n = *(b = newmap->table + tb_index);
		    } else {
#ifdef PACKAGE_MUDLIB_STATS
			add_array_size(&newmap->stats, count << 1);
#endif
			total_mapping_size += count * sizeof(mapping_node_t);
			total_mapping_nodes += count;
			newmap->count = count;
			error("Out of memory in filter_mapping\n");
		    }
		}
		if (++count > MAX_MAPPING_SIZE){
		    count--;
#ifdef PACKAGE_MUDLIB_STATS
		    add_array_size(&newmap->stats, count << 1);
#endif
		    total_mapping_size += count * sizeof(mapping_node_t);
                    total_mapping_nodes += count;
		    newmap->count = count;
		    mapping_too_large();
		}

		newnode = new_map_node();
		assign_svalue_no_free(newnode->values, elt->values);
		assign_svalue_no_free(newnode->values+1, elt->values+1);
		newnode->hashval = elt->hashval;
		(*b = newnode)->next = n;
	    }
	}
    } while (j--);

    if (count){
#ifdef PACKAGE_MUDLIB_STATS
        add_array_size(&newmap->stats, count << 1);
#endif
        total_mapping_size += count * sizeof(mapping_node_t);
        total_mapping_nodes += count;
	newmap->count += count;
    }

    sp--;
    pop_n_elems(num_arg);
    (++sp)->type = T_MAPPING;	
    sp->u.map = newmap;
}
#endif

/* compose_mapping */

INLINE mapping_t *
compose_mapping P3(mapping_t *,m1, mapping_t *,m2, unsigned short,flag)
{
	mapping_node_t *elt, *elt2, **a, **b = m2->table, **prev;
	unsigned short j = m1->table_size, deleted = 0;
	unsigned short mask = m2->table_size;
	svalue_t *sv;

	debug(1,("mapping.c: compose_mapping\n"));
	if (flag) m1 = copyMapping(m1);
	a = m1->table;

	do {
	    if ((elt = *(prev = a))) {
		do {
		    sv = elt->values + 1;
		    if ((elt2 = b[svalue_to_int(sv) & mask])) {
			do {
			    if (sameval(sv, elt2->values)){
				assign_svalue(sv, elt2->values + 1);
				break;
			    }
			} while ((elt2 = elt2->next));
		    }
		    if (!elt2){
			if (!(*prev = elt->next) && !(*a)) m1->unfilled++;
			deleted++;
			free_svalue(sv--, "compose_mapping");
			free_svalue(sv, "compose_mapping");
			free_node(elt);
		    }
		    prev = &(elt->next);
		} while ((elt = elt->next));
	    }
	} while (a++, j--);


	if (deleted){
	    m1->count -= deleted;
	    total_mapping_nodes -= deleted;
	    total_mapping_size -= deleted * sizeof(mapping_node_t);
	}

	if (flag) return m1;

	return NULL;
}

/* mapping_indices */

array_t *
mapping_indices P1(mapping_t *,m)
{
	array_t *v;
	int j = m->table_size;
	mapping_node_t *elt, **a = m->table;
	svalue_t *sv;

	debug(128,("mapping_indices: size = %d\n",m->count));

	v = allocate_empty_array(m->count);
	sv = v->item;
	do {
	    for (elt = a[j]; elt; elt = elt->next)
		assign_svalue_no_free(sv++, elt->values);
	} while (j--);
	return v;
}

/* mapping_values */

array_t *
mapping_values P1(mapping_t *,m)
{
	array_t *v;
	int j = m->table_size;
	mapping_node_t *elt, **a = m->table;
	svalue_t *sv;

	debug(128,("mapping_indices: size = %d\n",m->count));

	v = allocate_empty_array(m->count);
	sv = v->item;
	do {
	    for (elt = a[j]; elt; elt = elt->next)
		assign_svalue_no_free(sv++, elt->values + 1);
	} while (j--);
	return v;
}

#ifdef EACH
/* mapping_each */

INLINE array_t *
mapping_each P1(mapping_t *,m)
{
	int j;
	array_t *v;

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
		m->elt = (mapping_node_t *)0;
	}
	if (!m->elt) { /* find next occupied bucket in hash table */
		int found = 0;
		int k = (int) m->table_size;
	        mapping_node_t **a = m->table;

		for (j = m->bucket; j <= k; j++) {
			if (a[j]) {
				m->bucket = j + 1;
				m->elt = m->table[j];
				found = 1;
				break;
			}
		}
		if (!found) {
			m->bucket = 0;
			m->elt = (mapping_node_t *)0;
			return null_array();  /* have reached the end */
		}
	}
	v = allocate_empty_array(2);
	assign_svalue_no_free(v->item, m->elt->values);
	assign_svalue_no_free(v->item + 1, m->elt->values + 1);
	m->elt = m->elt->next;
	return v;
}
#endif

/* functions for building mappings */

static svalue_t *insert_in_mapping P2(mapping_t *, m, char *, key) {
    svalue_t lv;
    
    lv.type = T_STRING;
    lv.subtype = STRING_CONSTANT;
    lv.u.string = key;
    return find_for_insert(m, &lv, 1);
}

void add_mapping_pair P3(mapping_t *, m, char *, key, int, value)
{
    svalue_t *s;

    s = insert_in_mapping(m, key);
    s->type = T_NUMBER;
    s->subtype = 0;
    s->u.number = value;
}

void add_mapping_string P3(mapping_t *, m, char *, key, char *, value)
{
    svalue_t *s;

    s = insert_in_mapping(m, key);
    s->type = T_STRING;
    s->subtype = STRING_SHARED;
    s->u.string = make_shared_string(value);
}

void add_mapping_malloced_string P3(mapping_t *, m, char *, key, char *, value)
{
    svalue_t *s;

    s = insert_in_mapping(m, key);
    s->type = T_STRING;
    s->subtype = STRING_MALLOC;
    s->u.string = value;
}

void add_mapping_object P3(mapping_t *, m, char *, key, object_t *, value)
{
    svalue_t *s;

    s = insert_in_mapping(m, key);
    s->type = T_OBJECT;
    s->subtype = 0;
    s->u.ob = value;
    add_ref(value, "add_mapping_object");
}

void add_mapping_array P3(mapping_t *, m, char *, key, array_t *, value)
{
    svalue_t *s;

    s = insert_in_mapping(m, key);
    s->type = T_ARRAY;
    s->subtype = 0;
    s->u.arr = value;
    value->ref++;
}

void add_mapping_shared_string P3(mapping_t *, m, char *, key, char *, value)
{
    svalue_t *s;

    s = insert_in_mapping(m, key);
    s->type = T_STRING;
    s->subtype = STRING_SHARED;
    s->u.string = ref_string(value);
}
