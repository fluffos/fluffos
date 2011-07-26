/* 92/04/18 - cleaned up in accordance with ./src/style.guidelines */

#include "std.h"
#include "config.h"
#include "lpc_incl.h"
#include "md.h"
#include "efun_protos.h"

int num_mappings = 0;
int total_mapping_size = 0;
int total_mapping_nodes = 0;

mapping_node_t *locked_map_nodes = 0;

/*
 * LPC mapping (associative arrays) module.  Contains routines for
 * easy value and lvalue manipulation.
 *
 * Original binary tree version for LPCA written by one of the earliest MudOS
 * hackers.
 * - some enhancements by Truilkan@TMI
 * - rewritten for MudOS to use an extensible hash table implementation styled
 *   after the one Perl uses in hash.c - 92/07/08 - by Truilkan@TMI
 * - Beek reduced mem usage and improved speed 95/09/08; Sym optimized this
 *   at some point as well.
 */

/*
  growMap: based on hash.c:hsplit() from Larry Wall's Perl.

  growMap doubles the size of the hash table.  It is called when FILL_PERCENT
  of the buckets in the hash table contain values.  This routine is
  efficient since the entries in the table don't need to be rehashed (even
  though the entries are redistributed across the both halves of the hash
  table).
*/

INLINE_STATIC unsigned long node_hash (mapping_node_t * mn) {
    return MAP_SVAL_HASH(mn->values[0]);
}

INLINE int growMap (mapping_t * m)
{
        int oldsize = m->table_size + 1;
        int newsize = oldsize << 1;
        int i;
        mapping_node_t **a, **b, **eltp, *elt;

       /* resize the hash table to be twice the old size */
        m->table = a = RESIZE(m->table, newsize, mapping_node_t *, TAG_MAP_TBL, "growMap");
        if (!a) {
            /*
              We couldn't grow the hash table.  Rather than die, we just
              accept the performance hit resulting from having an overfull
              table.
              This trick won't work.  m->table is now zero. -Beek
              */
            m->unfilled = m->table_size;
            return 0;
        }
        /* hash table doubles in size -- keep track of the memory used */
        total_mapping_size += sizeof(mapping_node_t *) * oldsize;
        debug(mapping,("mapping.c: growMap ptr = %p, size = %d\n", m, newsize));
        m->unfilled = oldsize * (unsigned)FILL_PERCENT / (unsigned)100;
        m->table_size = newsize - 1;
        /* zero out the new storage area (2nd half of table) */
        memset(a += oldsize, 0, oldsize * sizeof(mapping_node_t *));
        i = oldsize;
        while (a--, i--) {
            if ((elt = *a)) {
                eltp = a, b = a + oldsize;
                do {
                    if (node_hash(elt) & oldsize) {
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

INLINE mapping_t *mapTraverse (mapping_t *m, int (*func) (mapping_t *, mapping_node_t *, void *), void *extra){
        mapping_node_t *elt, *nelt;
        int j = m->table_size;

        debug(mapping,("mapTraverse %p\n", m));
        do {
            for (elt = m->table[j]; elt; elt = nelt) {
                nelt = elt->next;
                if ((*func)(m, elt, extra)) return m;
            }
        } while (j--);
        return m;
}

/* free_mapping */

INLINE void
dealloc_mapping (mapping_t * m)
{
        debug(mapping,("mapping.c: actual free of %p\n", m));
        num_mappings--;
        {
            int j = m->table_size, c = MAP_COUNT(m);
            mapping_node_t *elt, *nelt, **a = m->table;

            total_mapping_size -= (sizeof(mapping_t) +
                                   sizeof(mapping_node_t *) * (j+1) +
                                   sizeof(mapping_node_t) * c);
            total_mapping_nodes -= c;
#ifdef PACKAGE_MUDLIB_STATS
            add_array_size (&m->stats, - (c << 1));
#endif

            do {
                for (elt = a[j]; elt; elt = nelt) {
                    nelt = elt->next;
                    free_svalue(elt->values, "free_mapping");
                    free_node(m, elt);
                }
            } while (j--);


            debug(mapping, ("in free_mapping: before table\n"));
            FREE((char *)a);
        }

        debug(mapping, ("in free_mapping: after table\n"));
        FREE((char *) m);
        debug(mapping, ("in free_mapping: after m\n"));
        debug(mapping,("mapping.c: free_mapping end\n"));
}

INLINE void
free_mapping (mapping_t * m)
{
        debug(mapping,("mapping.c: free_mapping begin, ptr = %p\n", m));
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

void unlock_mapping (mapping_t * m) {
    mapping_node_t **mn = &locked_map_nodes;
    mapping_node_t *tmp;

    while (*mn) {
        if ((*mn)->values[0].u.map == m) {
            free_svalue((*mn)->values + 1, "free_locked_nodes");
            /* take it out of the locked list ... */
            tmp = *mn;
            *mn = (*mn)->next;
            /* and add it to the free list */
            tmp->next = free_nodes;
            free_nodes = tmp;
        } else
            mn = &((*mn)->next);
    }
    m->count &= ~MAP_LOCKED;
}

void free_node (mapping_t * m, mapping_node_t * mn) {
    if (m->count & MAP_LOCKED) {
        mn->next = locked_map_nodes;
        locked_map_nodes = mn;
        mn->values[0].u.map = m;
    } else {
        free_svalue(mn->values + 1, "free_node");
        mn->next = free_nodes;
        free_nodes = mn;
    }
}

/* allocate_mapping(int n)

   call with n == 0 if you will be doing many deletions from the map.
   call with n == "approx. # of insertions you plan to do" if you won't be
   doing many deletions from the map.
*/

INLINE mapping_t *
allocate_mapping (int n)
{
        mapping_t *newmap;
        mapping_node_t **a;

        if (n > MAX_MAPPING_SIZE) n = MAX_MAPPING_SIZE;
        newmap = ALLOCATE(mapping_t, TAG_MAPPING, "allocate_mapping: 1");
        debug(mapping,("mapping.c: allocate_mapping begin, newmap = %p\n", newmap));
        if (newmap == NULL)
            error("Allocate_mapping - out of memory.\n");

        if (n > MAP_HASH_TABLE_SIZE) {
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
#ifdef PACKAGE_MUDLIB_STATS
        if (current_object) {
          assign_stats (&newmap->stats, current_object);
          add_array_size (&newmap->stats, n << 1);
        } else {
          null_stats (&newmap->stats);
        }
#endif
        num_mappings++;
        debug(mapping,("mapping.c: allocate_mapping end\n"));
        return newmap;
}

INLINE mapping_t *
allocate_mapping2 (array_t * arr, svalue_t * sv)
{
    mapping_t *newmap;
    int i;

    newmap = allocate_mapping(arr->size);
    push_refed_mapping(newmap);

    for (i = 0; i < arr->size; i++) {
        svalue_t *svp, *ret;

        svp = find_for_insert(newmap, arr->item + i, 1);
        if (sv->type == T_FUNCTION) {
            push_svalue(arr->item + i);
            ret = call_function_pointer(sv->u.fp, 1);
            *svp = *ret;
            ret->type = T_NUMBER;
        } else {
            assign_svalue_no_free(svp, sv);
        }
    }

    sp--;
    return newmap;
}

INLINE mapping_t *
mkmapping (array_t * k, array_t * v) {
    mapping_t *newmap;
    int i;

    newmap = allocate_mapping(k->size);
    for (i = 0; i < k->size; i++) {
        svalue_t *svp;

        svp = find_for_insert(newmap, k->item + i, 1);
        assign_svalue_no_free(svp, v->item + i);
    }

    return newmap;
}

/*
  copyMapping: make a copy of a mapping
*/

INLINE_STATIC mapping_t *
copyMapping (mapping_t *m)
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
    if (!c) {
        FREE((char *) newmap);
        error("copyMapping 2 - out of memory.\n");
    }
    newmap->count = m->count;
    total_mapping_nodes += MAP_COUNT(m);
    memset(c, 0, k * sizeof(mapping_node_t *));
    total_mapping_size += (sizeof(mapping_t) +
                           sizeof(mapping_node_t *) * k +
                           sizeof(mapping_node_t) * MAP_COUNT(m));

#ifdef PACKAGE_MUDLIB_STATS
    if (current_object) {
        assign_stats (&newmap->stats, current_object);
        add_array_size (&newmap->stats, MAP_COUNT(m) << 1);
    }
    else null_stats (&newmap->stats);
#endif
    num_mappings++;
    while (k--) {
        if ((elt = b[k])) {
            a = c + k;
            do {
                nelt = new_map_node();

                assign_svalue_no_free(nelt->values, elt->values);
                assign_svalue_no_free(nelt->values + 1, elt->values + 1);
                nelt->next = *a;
                *a = nelt;
            } while ((elt = elt->next));
        }
    }
    return newmap;
}

INLINE int
restore_hash_string (char ** val, svalue_t * sv)
{
    register char *cp = *val;
    char c, *start = cp;

    while ((c = *cp++) != '"') {
        switch(c) {
        case '\r':
            *(cp-1) = '\n';
            break;

        case '\\':
            {
                char *news = cp - 1;

                if ((c = *news++ = *cp++)) {
                    while ((c = *cp++) != '"') {
                        if (c == '\\') {
                            if (!(c = *news++ = *cp++)) return ROB_STRING_ERROR;
                        }
                        else {
                            if (c == '\r')
                                c = *news++ = '\n';
                            else *news++ = c;
                        }
                    }
                    if (!c) return ROB_STRING_ERROR;
                    *news = '\0';
                    *val = cp;
                    sv->u.string = make_shared_string(start);
                    sv->type = T_STRING;
                    sv->subtype = STRING_SHARED;
                    return 0;
                }
                else return ROB_STRING_ERROR;
            }

        case '\0':
            return ROB_STRING_ERROR;
        }
    }
    *val = cp;
    *--cp = '\0';
    sv->u.string = make_shared_string(start);
    sv->type = T_STRING;
    sv->subtype = STRING_SHARED;
    return 0;
}

/*
 * svalue_t_to_int: Converts an svalue into an integer index.
 */

int svalue_to_int (svalue_t *v)
{
    if (v->type == T_STRING && v->subtype != STRING_SHARED) {
        char *p = make_shared_string(v->u.string);
        free_string_svalue(v);
        v->subtype = STRING_SHARED;
        v->u.string = p;
    }
    //need to make it shared or all the assumptions about string==other string only when addresses match will fail!
    /* The bottom bits of pointers tend to be bad ...
     * Note that this means close groups of numbers don't hash particularly
     * well, but then one wonders why they aren't using an array ...
     */
    return MAP_SVAL_HASH(*v);
}

int msameval (svalue_t * arg1, svalue_t * arg2) {
	return (arg1->u.number == arg2->u.number);
    /* it's a union so those all do the same anyway! switch (arg1->type | arg2->type) {
    case T_NUMBER:
        return arg1->u.number == arg2->u.number;
    case T_REAL:
        return arg1->u.real == arg2->u.real;
    default:
        return arg1->u.arr == arg2->u.arr;
    }*/
}

/*
   mapping_delete: delete an element from the mapping
*/

INLINE void mapping_delete (mapping_t *m, svalue_t *lv)
{
        int i = svalue_to_int(lv) & m->table_size;
        mapping_node_t **prev = m->table + i, *elt;

        if ((elt = *prev)) {
            do {
                if (msameval(elt->values, lv)) {
                    if (!(*prev = elt->next) && !m->table[i]) {
                        m->unfilled++;
                        debug(mapping,("mapping delete: bucket empty, unfilled = %i",
                                    m->unfilled));
                    }
                    m->count--;
                    total_mapping_nodes--;
                    total_mapping_size -= sizeof(mapping_node_t);
                    debug(mapping,("mapping delete: count = %i", MAP_COUNT(m)));
                    free_svalue(elt->values, "mapping_delete");
                    free_node(m, elt);
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
find_for_insert (mapping_t * m, svalue_t * lv, int doTheFree)
{
        int oi = svalue_to_int(lv);
        unsigned int i = oi & m->table_size;
        mapping_node_t *n, *newnode, **a = m->table + i;

        debug(mapping,("mapping.c: hashed to %d\n", i));
        if ((n = *a)) {
            do {
                if (msameval(lv, n->values)) {
                    /* normally, the f_assign would free the old value */
                    debug(mapping,("mapping.c: found %p\n", n->values));
                    if (doTheFree) free_svalue(n->values + 1, "find_for_insert");
                    return n->values + 1;
                }
            } while ((n = n->next));
            debug(mapping,("mapping.c: didn't find %p\n", lv));
            n = *a;
        }
        else if (!(--m->unfilled)) {
            int size = m->table_size + 1;

            if (growMap(m)) {
                if (oi & size) i |= size;
                n = *(a = m->table + i);
            } else {
                error("Out of memory\n");
            }
        }

        m->count++;
        if (MAP_COUNT(m) > MAX_MAPPING_SIZE) {
            m->count--;
            debug(mapping,("mapping.c: too full"));
            mapping_too_large();
        }
#ifdef PACKAGE_MUDLIB_STATS
        add_array_size (&m->stats, 2);
#endif
        total_mapping_size += sizeof(mapping_node_t);
        debug(mapping,("mapping.c: allocated a node\n"));
        newnode = new_map_node();
        assign_svalue_no_free(newnode->values, lv);
        /* insert at head of bucket */
        (*a = newnode)->next = n;
        lv = newnode->values + 1;
        *lv = const0u;
        total_mapping_nodes++;
        return lv;
}

#ifdef F_UNIQUE_MAPPING

typedef struct unique_node_s {
    svalue_t key;
    int count;
    struct unique_node_s *next;
    int *indices;
} unique_node_t;

typedef struct unique_m_list_s {
    unique_node_t **utable;
    struct unique_m_list_s *next;
    unsigned int mask;
} unique_m_list_t;

static unique_m_list_t *g_u_m_list = 0;

static void unique_mapping_error_handler (void)
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

void f_unique_mapping (void)
{
    unique_m_list_t *nlist;
    svalue_t *arg = sp - st_num_arg + 1, *sv;
    unique_node_t **table, *uptr, *nptr;
    array_t *v = arg->u.arr, *ret;
    unsigned int oi, i, numkeys = 0, mask, size;
    unsigned short num_arg = st_num_arg;
    unsigned int nmask;
    mapping_t *m;
    mapping_node_t **mtable, *elt;
    int *ind, j;
    function_to_call_t ftc;

    process_efun_callback(1, &ftc, F_UNIQUE_MAPPING);

    size = v->size;
    if (!size) {
        pop_n_elems(num_arg - 1);
        free_array(v);
        sp->type = T_MAPPING;
        sp->u.map = allocate_mapping(0);
        return;
    }

    if (size > MAP_HASH_TABLE_SIZE) {
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

    STACK_INC;
    sp->type = T_ERROR_HANDLER;
    sp->u.error_handler = unique_mapping_error_handler;

    size = v->size;
    while (size--) {
        push_svalue(v->item + size);
        sv = call_efun_callback(&ftc, 1);
        if(sv)
        	i = (oi = svalue_to_int(sv)) & mask;
        else
        	i = oi = 0;
        if ((uptr = table[i])) {
            do {
                if (msameval(&uptr->key, sv)) {
                    ind = uptr->indices = RESIZE(uptr->indices, uptr->count+1,
                                                 int, 102, "f_unique_mapping:3");
                    ind[uptr->count++] = size;
                    break;
                }
            } while ((uptr = uptr->next));
        }
        if (!uptr) {
            uptr = ALLOCATE(unique_node_t, 103, "f_unique_mapping:4");
            assign_svalue_no_free(&uptr->key, sv);
            uptr->count = 1;
            uptr->indices = ALLOCATE(int, 104, "f_unique_mapping:5");
            uptr->indices[0] = size;
            uptr->next = table[i];
            table[i] = uptr;
            numkeys++;
        }
    }

    m = allocate_mapping(nmask = numkeys << 1);
    mtable = m->table;
    numkeys = 0;

    if (nmask > MAP_HASH_TABLE_SIZE) {
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
                oi = MAP_SVAL_HASH(uptr->key);
                i = oi & nmask;
                if (!mtable[i] && !(--m->unfilled)) {
                    if (growMap(m)) {
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
                *elt->values = uptr->key;
                (elt->values + 1)->type = T_ARRAY;
                ret = (elt->values + 1)->u.arr = allocate_empty_array(size = uptr->count);
                ind = uptr->indices;
                while (size--) {
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
load_mapping_from_aggregate (svalue_t *sp, int n)
{
        mapping_t *m;
        int mask, i, oi, count = 0;
        mapping_node_t **a, *elt, *elt2;

        debug(mapping,("mapping.c: load_mapping_from_aggregate begin, size = %d\n", n));
        m = allocate_mapping(n >> 1);
        if (!n) return m;
        mask = m->table_size;
        a = m->table;
        do {
            i = (oi = svalue_to_int(++sp)) & mask;
            if ((elt2 = elt = a[i])) {
                do {
                    if (msameval(sp, elt->values)) {
                        free_svalue(sp++, "load_mapping_from_aggregate: duplicate key");
                        free_svalue(elt->values+1, "load_mapping_from_aggregate");
                        *(elt->values+1) = *sp;
                        break;
                    }
                } while ((elt = elt->next));
                if (elt) continue;
            }
            else if (!(--m->unfilled)) {
                if (growMap(m)) {
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

            if (++count > MAX_MAPPING_SIZE) {
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
            (a[i] = elt)->next = elt2;
        } while (n -= 2);
#ifdef PACKAGE_MUDLIB_STATS
        add_array_size(&m->stats, count << 1);
#endif
        total_mapping_size += sizeof(mapping_node_t) * (m->count = count);
        total_mapping_nodes += count;
        debug(mapping,("mapping.c: load_mapping_from_aggregate end\n"));
        return m;
}

/* is ok */

INLINE svalue_t *
find_in_mapping (mapping_t * m, svalue_t *lv)
{
        int i = svalue_to_int(lv) & m->table_size;
        mapping_node_t *n = m->table[i];

        while (n) {
            if (msameval(n->values, lv)) return n->values + 1;
            n = n->next;
        }

        return &const0u;
}

svalue_t *
find_string_in_mapping (mapping_t * m, const char * p)
{
    char *ss = findstring(p);
    int i;
    mapping_node_t *n;
    static svalue_t str = {T_STRING, STRING_SHARED};
    if (!ss) return &const0u;
    str.u.string = ss;
    i = MAP_SVAL_HASH(str);
    n = m->table[i & m->table_size];

    while (n) {
        if (n->values->type == T_STRING && n->values->u.string == ss)
            return n->values + 1;
        n = n->next;
    }
    return &const0u;
}

/*
    add_to_mapping: adds mapping m2 to m1
*/

INLINE_STATIC void
add_to_mapping (mapping_t *m1, mapping_t *m2, int free_flag)
{
    int mask = m1->table_size, j = m2->table_size;
    int count = MAP_COUNT(m1);
    int i, oi;
    mapping_node_t *elt1, *elt2, *newnode, *n;
    mapping_node_t **a1 = m1->table, **a2 = m2->table;
    svalue_t *sv;

    do {
        for (elt2 = a2[j]; elt2; elt2 = elt2->next) {
            i = (oi = node_hash(elt2)) & mask;
            sv = elt2->values;
            if ((n = elt1 = a1[i])) {
                do {
                    if (msameval(sv, elt1->values)) {
                        assign_svalue(elt1->values + 1, sv + 1);
                        break;
                    }
                } while ((elt1 = elt1->next));
                if (elt1) continue;
            } else if (!(--m1->unfilled)) {
                if (growMap(m1)) {
                    a1 = m1->table;
                    if (oi & ++mask) n = a1[i |= mask];
                    mask <<= 1;
                    mask--;
                } else{
                    count -= MAP_COUNT(m1);
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
            count++;
            if (count > MAX_MAPPING_SIZE) {
                if (count -= MAP_COUNT(m1) + 1) {
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
            (a1[i] = newnode)->next = n;
        }
    } while (j--);

    if (count -= MAP_COUNT(m1)) {
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

INLINE_STATIC void
unique_add_to_mapping (mapping_t *m1, mapping_t *m2, int free_flag)
{
    int mask = m1->table_size, j = m2->table_size;
    int count = MAP_COUNT(m1);
    int i, oi;
    mapping_node_t *elt1, *elt2, *newnode, *n;
    mapping_node_t **a1 = m1->table, **a2 = m2->table;
    svalue_t *sv;

    do {
        for (elt2 = a2[j]; elt2; elt2 = elt2->next) {
            i = (oi = node_hash(elt2)) & mask;
            sv = elt2->values;
            if ((n = elt1 = a1[i])) {
                do {
                    if (msameval(sv, elt1->values)) break;
                } while ((elt1 = elt1->next));
                if (elt1) continue;
            }
            else if (!(--m1->unfilled)) {
                if (growMap(m1)) {
                    a1 = m1->table;
                    if (oi & ++mask) n = a1[i |= mask];
                    mask <<= 1;
                    mask--;
                } else{
                    ++m1->unfilled;
                    count -= MAP_COUNT(m1);
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

            if (++count > MAX_MAPPING_SIZE) {
                if (count -= MAP_COUNT(m1) + 1) {
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
            (a1[i] = newnode)->next = n;
        }
    } while (j--);

    if (count -= MAP_COUNT(m1)) {
#ifdef PACKAGE_MUDLIB_STATS
        add_array_size(&m1->stats, count << 1);
#endif
        total_mapping_size += count * sizeof(mapping_node_t);
        total_mapping_nodes += count;
    }

    m1->count += count;
}

INLINE void absorb_mapping(mapping_t *m1, mapping_t *m2){
    if (MAP_COUNT(m2)) {
        if (m1 != m2)
            add_to_mapping(m1, m2, 0);
    }
}

/*
   add_mapping: returns a new mapping that contains everything
   in two old mappings.  (uses hash table)
*/

INLINE mapping_t *
add_mapping (mapping_t *m1, mapping_t *m2)
{
        mapping_t *newmap;

        debug(mapping,("mapping.c: add_mapping begin: %p, %p", m1, m2));
        if (MAP_COUNT(m1) >= MAP_COUNT(m2)) {
            if (MAP_COUNT(m2)) {
                add_to_mapping(newmap = copyMapping(m1), m2, 1);
                return newmap;
            }
            else return copyMapping(m1);
        }
        else if (MAP_COUNT(m1)) {
            unique_add_to_mapping(newmap = copyMapping(m2), m1, 1);
            return newmap;
        }
        else return copyMapping(m2);
        debug(mapping,("mapping.c: add_mapping end\n"));
}

/*
   map_mapping: A lot of the efuns that work on arrays, such as
   filter_array(), should also work on mappings.
*/

#ifdef F_MAP
void
map_mapping (svalue_t * arg, int num_arg)
{
    mapping_t *m;
    mapping_node_t **a, *elt;
    int j;
    svalue_t *ret;
    function_to_call_t ftc;

    process_efun_callback(1, &ftc, F_MAP);

    if (arg->u.map->ref > 1) {
        m = copyMapping(arg->u.map);
        free_mapping(arg->u.map);
        arg->u.map = m;
    } else {
        m = arg->u.map;
    }

    j = m->table_size;
    a = m->table;
    debug(mapping,("mapping.c: map_mapping\n"));
    do {
        for (elt = a[j]; elt ; elt = elt->next) {
            push_svalue(elt->values);
            push_svalue(elt->values+1);
            ret = call_efun_callback(&ftc, 2);
            if (ret) assign_svalue(elt->values+1, ret);
            else break;
        }
    } while (j--);

    pop_n_elems(num_arg-1);
}
#endif

#ifdef F_FILTER
void
filter_mapping (svalue_t * arg, int num_arg)
{
    mapping_t *m, *newmap;
    mapping_node_t **a, *elt;
    mapping_node_t **b, *newnode, *n;
    int j, count = 0, size;
    svalue_t *ret;
    unsigned int tb_index;
    function_to_call_t ftc;

    process_efun_callback(1, &ftc, F_FILTER);

    if (arg->u.map->ref > 1) {
        m = copyMapping(arg->u.map);
        free_mapping(arg->u.map);
        arg->u.map = m;
    } else {
        m = arg->u.map;
    }

    newmap = allocate_mapping(0);
    push_refed_mapping(newmap);
    b = newmap->table;
    size = newmap->table_size;

    a = m->table;
    j = m->table_size;
    debug(mapping,("mapping.c: filter_mapping\n"));
    do {
        for (elt = a[j]; elt ; elt = elt->next) {
            push_svalue(elt->values);
            push_svalue(elt->values+1);
            ret = call_efun_callback(&ftc, 2);
            if (!ret) break;
            else if (ret->type != T_NUMBER || ret->u.number) {
                tb_index = node_hash(elt) & size;
                b = newmap->table + tb_index;
                if (!(n = *b) && !(--newmap->unfilled)) {
                    if (growMap(newmap)) {
                        size = newmap->table_size;
                        tb_index = node_hash(elt) & size;
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
                if (++count > MAX_MAPPING_SIZE) {
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
                (*b = newnode)->next = n;
            }
        }
    } while (j--);

    if (count) {
#ifdef PACKAGE_MUDLIB_STATS
        add_array_size(&newmap->stats, count << 1);
#endif
        total_mapping_size += count * sizeof(mapping_node_t);
        total_mapping_nodes += count;
        newmap->count += count;
    }

    sp--;
    pop_n_elems(num_arg);
    push_refed_mapping(newmap);
}
#endif

/* compose_mapping */

INLINE mapping_t *
compose_mapping (mapping_t *m1, mapping_t *m2, unsigned short flag)
{
    mapping_node_t *elt, *elt2, **a, **b = m2->table, **prev;
    unsigned int j = m1->table_size, deleted = 0;
    unsigned int mask = m2->table_size;
    svalue_t *sv;

    debug(mapping,("mapping.c: compose_mapping\n"));
    if (flag)
        m1 = copyMapping(m1);
    a = m1->table;

    do {
        if ((elt = *(prev = a))) {
            do {
                sv = elt->values + 1;
                if ((elt2 = b[svalue_to_int(sv) & mask])) {
                    do {
                        if (msameval(sv, elt2->values)) {
                            if (sv != elt2->values + 1) /* if m1 == m2 */
                                assign_svalue(sv, elt2->values + 1);
                            break;
                        }
                    } while ((elt2 = elt2->next));
                }
                if (!elt2) {
                    if (!(*prev = elt->next) && !(*a)) m1->unfilled++;
                    deleted++;
                    free_node(m1, elt);
                } else {
                    prev = &(elt->next);
                }
            } while ((elt = *prev));
        }
    } while (a++, j--);

    if (deleted) {
        m1->count -= deleted;
        total_mapping_nodes -= deleted;
        total_mapping_size -= deleted * sizeof(mapping_node_t);
    }

    return m1;
}

/* mapping_indices */

array_t *
mapping_indices (mapping_t *m)
{
        array_t *v;
        int j = m->table_size;
        mapping_node_t *elt, **a = m->table;
        svalue_t *sv;

        debug(mapping,("mapping_indices: size = %d\n", MAP_COUNT(m)));

        v = allocate_empty_array(MAP_COUNT(m));
        sv = v->item;
        do {
            for (elt = a[j]; elt; elt = elt->next)
                assign_svalue_no_free(sv++, elt->values);
        } while (j--);
        return v;
}

/* mapping_values */

array_t *
mapping_values (mapping_t *m)
{
        array_t *v;
        int j = m->table_size;
        mapping_node_t *elt, **a = m->table;
        svalue_t *sv;

        debug(mapping,("mapping_values: size = %d\n",MAP_COUNT(m)));

        v = allocate_empty_array(MAP_COUNT(m));
        sv = v->item;
        do {
            for (elt = a[j]; elt; elt = elt->next)
                assign_svalue_no_free(sv++, elt->values + 1);
        } while (j--);
        return v;
}

/* functions for building mappings */

static svalue_t *insert_in_mapping (mapping_t * m, const char * key) {
    svalue_t lv;
    svalue_t *ret;

    lv.type = T_STRING;
    lv.subtype = STRING_CONSTANT;
    lv.u.string = key;
    ret = find_for_insert(m, &lv, 1);
    /* lv.u.string will have been converted to a shared string */
    free_string(lv.u.string);
    return ret;
}

void add_mapping_pair (mapping_t * m, const char * key, long value)
{
    svalue_t *s;

    s = insert_in_mapping(m, key);
    s->type = T_NUMBER;
    s->subtype = 0;
    s->u.number = value;
}

void add_mapping_string (mapping_t * m, const char * key, const char * value)
{
    svalue_t *s;

    s = insert_in_mapping(m, key);
    s->type = T_STRING;
    s->subtype = STRING_SHARED;
    s->u.string = make_shared_string(value);
}

void add_mapping_malloced_string (mapping_t * m, const char * key, char * value)
{
    svalue_t *s;

    s = insert_in_mapping(m, key);
    s->type = T_STRING;
    s->subtype = STRING_MALLOC;
    s->u.string = value;
}

void add_mapping_object (mapping_t * m, const char * key, object_t * value)
{
    svalue_t *s;

    s = insert_in_mapping(m, key);
    s->type = T_OBJECT;
    s->subtype = 0;
    s->u.ob = value;
    add_ref(value, "add_mapping_object");
}

void add_mapping_array (mapping_t * m, const char * key, array_t * value)
{
    svalue_t *s;

    s = insert_in_mapping(m, key);
    s->type = T_ARRAY;
    s->subtype = 0;
    s->u.arr = value;
    value->ref++;
}

void add_mapping_shared_string (mapping_t * m, const char * key, char * value)
{
    svalue_t *s;

    s = insert_in_mapping(m, key);
    s->type = T_STRING;
    s->subtype = STRING_SHARED;
    s->u.string = ref_string(value);
}
