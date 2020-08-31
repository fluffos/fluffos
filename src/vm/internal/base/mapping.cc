/* 92/04/18 - cleaned up in accordance with ./src/style.guidelines */

#include "base/std.h"

#include <deque>
#include <map>

#include "vm/internal/base/machine.h"

mapping_node_t *locked_map_nodes = nullptr;

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

LPC_INT sval_hash(svalue_t x) {
  switch (x.type) {
    case T_STRING:
      return HASH(BLOCK(x.u.string));
    case T_NUMBER:
      return static_cast<unsigned long>(x.u.number);
    case T_OBJECT:
    // return HASH(BLOCK(x.u.ob->obname));
    default:
      return ((((x).u.number)) >> 5);
  }
}
/*
  growMap: based on hash.c:hsplit() from Larry Wall's Perl.

  growMap doubles the size of the hash table.  It is called when FILL_PERCENT
  of the buckets in the hash table contain values.  This routine is
  efficient since the entries in the table don't need to be rehashed (even
  though the entries are redistributed across the both halves of the hash
  table).
*/

static unsigned long node_hash(mapping_node_t *mn) { return MAP_SVAL_HASH(mn->values[0]); }

int growMap(mapping_t *m) {
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
  debug(mapping, "mapping.c: growMap ptr = %p, size = %d\n", (void *)m, newsize);
  m->unfilled = oldsize * static_cast<unsigned>(FILL_PERCENT) / static_cast<unsigned>(100);
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
          if (!(elt->next = *b)) {
            m->unfilled--;
          }
          *b = elt;
          elt = *eltp;
        } else {
          elt = *(eltp = &elt->next);
        }
      } while (elt);
      if (!*a) {
        m->unfilled++;
      }
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

mapping_t *mapTraverse(mapping_t *m, int (*func)(mapping_t *, mapping_node_t *, void *),
                       void *extra) {
  mapping_node_t *elt, *nelt;
  int j = m->table_size;

  debug(mapping, "mapTraverse %p\n", (void *)m);
  do {
    for (elt = m->table[j]; elt; elt = nelt) {
      nelt = elt->next;
      if ((*func)(m, elt, extra)) {
        return m;
      }
    }
  } while (j--);
  return m;
}

/* free_mapping */

void dealloc_mapping(mapping_t *m) {
  debug(mapping, "mapping.c: actual free of %p\n", (void *)m);
  num_mappings--;
  {
    int j = m->table_size, c = MAP_COUNT(m);
    mapping_node_t *elt, *nelt, **a = m->table;

    total_mapping_size -=
        (sizeof(mapping_t) + sizeof(mapping_node_t *) * (j + 1) + sizeof(mapping_node_t) * c);
    total_mapping_nodes -= c;
#ifdef PACKAGE_MUDLIB_STATS
    add_array_size(&m->stats, -(c << 1));
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
  FREE((char *)m);
  debug(mapping, ("in free_mapping: after m\n"));
  debug(mapping, ("mapping.c: free_mapping end\n"));
}

void free_mapping(mapping_t *m) {
  debug(mapping, "mapping.c: free_mapping begin, ptr = %p\n", (void *)m);
  /* some other object is still referencing this mapping */
  if (--m->ref > 0) {
    return;
  }
  dealloc_mapping(m);
}

static mapping_node_t *free_nodes = nullptr;
mapping_node_block_t *mapping_node_blocks = nullptr;

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
    mnb = reinterpret_cast<mapping_node_block_t *>(
        DMALLOC(sizeof(mapping_node_block_t), TAG_MAP_NODE_BLOCK, "new_map_node"));
    mnb->next = mapping_node_blocks;
    mapping_node_blocks = mnb;
    mnb->nodes[MNB_SIZE - 1].next = nullptr;
    for (i = MNB_SIZE - 1; i--;) {
      mnb->nodes[i].next = &mnb->nodes[i + 1];
    }
    ret = &mnb->nodes[0];
    free_nodes = &mnb->nodes[1];
  }
  return ret;
}

void unlock_mapping(mapping_t *m) {
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
    } else {
      mn = &((*mn)->next);
    }
  }
  m->count &= ~MAP_LOCKED;
}

void free_node(mapping_t *m, mapping_node_t *mn) {
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

mapping_t *allocate_mapping(int n) {
  mapping_t *newmap;
  mapping_node_t **a;

  if (n > MAX_MAPPING_SIZE) {
    n = MAX_MAPPING_SIZE;
  }
  newmap =
      reinterpret_cast<mapping_t *>(DMALLOC(sizeof(mapping_t), TAG_MAPPING, "allocate_mapping: 1"));
  debug(mapping, "mapping.c: allocate_mapping begin, newmap = %p\n", (void *)newmap);
  if (newmap == nullptr) {
    error("Allocate_mapping - out of memory.\n");
  }

  if (n > MAP_HASH_TABLE_SIZE) {
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    if (n & 0xff00) {
      n |= n >> 8;
    }
    newmap->table_size = n++;
  } else {
    newmap->table_size = (n = MAP_HASH_TABLE_SIZE) - 1;
  }
  /* The size is actually 1 higher */
  newmap->unfilled = n * static_cast<unsigned>(FILL_PERCENT) / static_cast<unsigned>(100);
  a = newmap->table = reinterpret_cast<mapping_node_t **>(
      DMALLOC(n *= sizeof(mapping_node_t *), TAG_MAP_TBL, "allocate_mapping: 3"));
  if (!a) {
    error("Allocate_mapping 2 - out of memory.\n");
  }
  /* zero out the hash table */
  memset(a, 0, n);
  total_mapping_size += sizeof(mapping_t) + n;
  newmap->ref = 1;
  newmap->count = 0;
#ifdef PACKAGE_MUDLIB_STATS
  if (current_object) {
    assign_stats(&newmap->stats, current_object);
    add_array_size(&newmap->stats, n << 1);
  } else {
    null_stats(&newmap->stats);
  }
#endif
  num_mappings++;
  debug(mapping, ("mapping.c: allocate_mapping end\n"));
  return newmap;
}

mapping_t *allocate_mapping2(array_t *arr, svalue_t *sv) {
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

mapping_t *mkmapping(array_t *k, array_t *v) {
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

static mapping_t *copyMapping(mapping_t *m) {
  mapping_t *newmap;
  int k = m->table_size;
  mapping_node_t *elt, *nelt, **a, **b = m->table, **c;

  newmap =
      reinterpret_cast<mapping_t *>(DMALLOC(sizeof(mapping_t), TAG_MAPPING, "copy_mapping: 1"));
  if (newmap == nullptr) {
    error("copyMapping - out of memory.\n");
  }
  newmap->table_size = k++;
  newmap->unfilled = m->unfilled;
  newmap->ref = 1;
  c = newmap->table = reinterpret_cast<mapping_node_t **>(
      DCALLOC(k, sizeof(mapping_node_t *), TAG_MAP_TBL, "copy_mapping: 2"));
  if (!c) {
    FREE((char *)newmap);
    error("copyMapping 2 - out of memory.\n");
  }
  newmap->count = m->count;
  total_mapping_nodes += MAP_COUNT(m);
  memset(c, 0, k * sizeof(mapping_node_t *));
  total_mapping_size +=
      (sizeof(mapping_t) + sizeof(mapping_node_t *) * k + sizeof(mapping_node_t) * MAP_COUNT(m));

#ifdef PACKAGE_MUDLIB_STATS
  if (current_object) {
    assign_stats(&newmap->stats, current_object);
    add_array_size(&newmap->stats, MAP_COUNT(m) << 1);
  } else {
    null_stats(&newmap->stats);
  }
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

int restore_hash_string(char **val, svalue_t *sv) {
  char *cp = *val;
  char c, *start = cp;

  while ((c = *cp++) != '"') {
    switch (c) {
      case '\r':
        *(cp - 1) = '\n';
        break;

      case '\\': {
        char *news = cp - 1;

        if ((c = *news++ = *cp++)) {
          while ((c = *cp++) != '"') {
            if (c == '\\') {
              if (!(c = *news++ = *cp++)) {
                return ROB_STRING_ERROR;
              }
            } else {
              if (c == '\r') {
                c = *news++ = '\n';
              } else {
                *news++ = c;
              }
            }
          }
          if (!c) {
            return ROB_STRING_ERROR;
          }
          *news = '\0';
          *val = cp;
          if (!u8_validate(start)) {
            return ROB_STRING_UTF8_ERROR;
          }
          sv->u.string = make_shared_string(start);
          sv->type = T_STRING;
          sv->subtype = STRING_SHARED;
          return 0;
        } else {
          return ROB_STRING_ERROR;
        }
      }

      case '\0':
        return ROB_STRING_ERROR;
    }
  }
  *val = cp;
  *--cp = '\0';
  if (!u8_validate(start)) {
    return ROB_STRING_UTF8_ERROR;
  }
  sv->u.string = make_shared_string(start);
  sv->type = T_STRING;
  sv->subtype = STRING_SHARED;
  return 0;
}

/*
 * svalue_t_to_int: Converts an svalue into an integer index.
 */

LPC_INT svalue_to_int(svalue_t *v) {
  if (v->type == T_STRING && v->subtype != STRING_SHARED) {
    char *p = make_shared_string(v->u.string);
    free_string_svalue(v);
    v->subtype = STRING_SHARED;
    v->u.string = p;
  }
  // need to make it shared or all the assumptions about string==other string
  // only when addresses match will fail!
  /* The bottom bits of pointers tend to be bad ...
   * Note that this means close groups of numbers don't hash particularly
   * well, but then one wonders why they aren't using an array ...
   */
  return MAP_SVAL_HASH(*v);
}

int msameval(svalue_t *arg1, svalue_t *arg2) {
  switch (arg1->type | arg2->type) {
    case T_NUMBER:
      return arg1->u.number == arg2->u.number;
    case T_REAL:
      return arg1->u.real == arg2->u.real;
    default:
      return arg1->u.arr == arg2->u.arr;
  }
}

/*
   mapping_delete: delete an element from the mapping
*/

void mapping_delete(mapping_t *m, svalue_t *lv) {
  int i = svalue_to_int(lv) & m->table_size;
  mapping_node_t **prev = m->table + i, *elt;

  if ((elt = *prev)) {
    do {
      if (msameval(elt->values, lv)) {
        if (!(*prev = elt->next) && !m->table[i]) {
          m->unfilled++;
          debug(mapping, "mapping delete: bucket empty, unfilled = %i", m->unfilled);
        }
        m->count--;
        total_mapping_nodes--;
        total_mapping_size -= sizeof(mapping_node_t);
        debug(mapping, "mapping delete: count = %i", MAP_COUNT(m));
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

svalue_t *find_for_insert(mapping_t *m, svalue_t *lv, int doTheFree) {
  int oi = svalue_to_int(lv);
  unsigned int i = oi & m->table_size;
  mapping_node_t *n, *newnode, **a = m->table + i;

  debug(mapping, "mapping.c: hashed to %d\n", i);
  if ((n = *a)) {
    do {
      if (msameval(lv, n->values)) {
        /* normally, the f_assign would free the old value */
        debug(mapping, "mapping.c: found %p\n", (void *)(n->values));
        if (doTheFree) {
          free_svalue(n->values + 1, "find_for_insert");
        }
        return n->values + 1;
      }
    } while ((n = n->next));
    debug(mapping, "mapping.c: didn't find %p\n", (void *)lv);
    n = *a;
  } else if (!(--m->unfilled)) {
    int size = m->table_size + 1;

    if (growMap(m)) {
      if (oi & size) {
        i |= size;
      }
      n = *(a = m->table + i);
    } else {
      error("Out of memory\n");
    }
  }

  m->count++;
  if (MAP_COUNT(m) > MAX_MAPPING_SIZE) {
    m->count--;
    debug(mapping, ("mapping.c: too full"));
    mapping_too_large();
  }
#ifdef PACKAGE_MUDLIB_STATS
  add_array_size(&m->stats, 2);
#endif
  total_mapping_size += sizeof(mapping_node_t);
  debug(mapping, ("mapping.c: allocated a node\n"));
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

struct unique_svalue_compare {
  bool operator()(svalue_t l, svalue_t r) const {
    DEBUG_CHECK(((l.type == T_STRING && l.subtype != STRING_SHARED) ||
                 (r.type == T_STRING && r.subtype != STRING_SHARED)),
                "unique_mapping: using non-shared string! "
                "Memory corruption will happen.");
    return svalue_to_int(&l) < svalue_to_int(&r);
  }
};

void f_unique_mapping(void) {
  svalue_t *arg = sp - st_num_arg + 1;
  array_t *v = arg->u.arr;
  unsigned short num_arg = st_num_arg;

  if (!v || !v->size) {
    pop_n_elems(num_arg);
    push_refed_mapping(allocate_mapping(0));
    return;
  }

  // prepare the call back.
  function_to_call_t ftc;
  process_efun_callback(1, &ftc, F_UNIQUE_MAPPING);

  // for each item in the array, call the callback and group
  // item with same result together.
  typedef std::map<svalue_t, std::deque<svalue_t *>, unique_svalue_compare> MapResult;
  MapResult result;
  int size = v->size;
  while (size--) {
    svalue_t *sv;
    push_svalue(v->item + size);
    sv = call_efun_callback(&ftc, 1);
    if (!sv) {
      sv = &const0;
    }

    // FIXME: special hack.
    // svalue_to_int will modify parameter to make sure it is shared string,
    // Thus we must call it here to "fix" sv before we insert it into map.
    //
    // The correct fix here should be change svalue_to_int to *not* modify
    // param and make sure all call site only pass shared string.
    svalue_to_int(sv);

    // If key is not in the map yet, we need to make a copy and insert it.
    //
    // Important: sv is pointing to a static global value and will be freed on
    // next efun call, since we need the value it contains,  we need to make a
    // deep copy.
    //
    // We also need to free the key when we done, it is done in the loop below.
    if (result.find(*sv) == result.end()) {
      svalue_t key;
      assign_svalue_no_free(&key, sv);

      result.insert(MapResult::value_type(key, std::deque<svalue_t *>()));
    }

    // NOTE: going through array in reverse order , but put the result in the
    // back of the array, this is to preserve the observed behavior of old
    // implementation.
    result.find(*sv)->second.push_back(v->item + size);
  }

  // Translate result into LPC mapping
  mapping_t *m = allocate_mapping(0);
  for (auto &item : result) {
    auto key = item.first;
    auto values = item.second;

    // FIXME: find_for_insert can actually throw error if we exceeded maximum
    // mapping size! we will leave garbage when that happens.
    //
    // key is copied, but not freed, the value is freed at the end of the loop.
    svalue_t *l = find_for_insert(m, &key, 0);
    l->type = T_ARRAY;
    l->u.arr = allocate_empty_array(values.size());
    for (unsigned int i = 0; i < values.size(); i++) {
      // values are copied.
      assign_svalue_no_free(&l->u.arr->item[i], values[i]);
    }
    // Free reference
    free_svalue(&key, "unique_mapping");
  }
  pop_n_elems(num_arg);
  push_refed_mapping(m);
}
#endif /* End of unique_mapping */

/*
 * load_mapping_from_aggregate: Create a new mapping, loading from an
 * array of svalues. Format of data: LHS RHS LHS2 RHS2... (uses hash table)
 */

mapping_t *load_mapping_from_aggregate(svalue_t *sp, int n) {
  mapping_t *m;
  int mask, i, oi, count = 0;
  mapping_node_t **a, *elt, *elt2;

  debug(mapping, "mapping.c: load_mapping_from_aggregate begin, size = %d\n", n);
  m = allocate_mapping(n >> 1);
  if (!n) {
    return m;
  }
  mask = m->table_size;
  a = m->table;
  do {
    i = (oi = svalue_to_int(++sp)) & mask;
    if ((elt2 = elt = a[i])) {
      do {
        if (msameval(sp, elt->values)) {
          free_svalue(sp++, "load_mapping_from_aggregate: duplicate key");
          free_svalue(elt->values + 1, "load_mapping_from_aggregate");
          *(elt->values + 1) = *sp;
          break;
        }
      } while ((elt = elt->next));
      if (elt) {
        continue;
      }
    } else if (!(--m->unfilled)) {
      if (growMap(m)) {
        a = m->table;
        if (oi & ++mask) {
          elt2 = a[i |= mask];
        }
        mask <<= 1;
        mask--;
      } else {
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
  debug(mapping, ("mapping.c: load_mapping_from_aggregate end\n"));
  return m;
}

/* is ok */

svalue_t *find_in_mapping(mapping_t *m, svalue_t *lv) {
  int i = svalue_to_int(lv) & m->table_size;
  mapping_node_t *n = m->table[i];

  while (n) {
    if (msameval(n->values, lv)) {
      return n->values + 1;
    }
    n = n->next;
  }

  return &const0u;
}

svalue_t *find_string_in_mapping(mapping_t *m, const char *p) {
  char *ss = findstring(p);
  int i;
  mapping_node_t *n;
  static svalue_t str = {T_STRING, STRING_SHARED};
  if (!ss) {
    return &const0u;
  }
  str.u.string = ss;
  i = MAP_SVAL_HASH(str);
  n = m->table[i & m->table_size];

  while (n) {
    if (n->values->type == T_STRING && n->values->u.string == ss) {
      return n->values + 1;
    }
    n = n->next;
  }
  return &const0u;
}

/*
    add_to_mapping: adds mapping m2 to m1
*/

static void add_to_mapping(mapping_t *m1, mapping_t *m2, int free_flag) {
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
        if (elt1) {
          continue;
        }
      } else if (!(--m1->unfilled)) {
        if (growMap(m1)) {
          a1 = m1->table;
          if (oi & ++mask) {
            n = a1[i |= mask];
          }
          mask <<= 1;
          mask--;
        } else {
          count -= MAP_COUNT(m1);
#ifdef PACKAGE_MUDLIB_STATS
          add_array_size(&m1->stats, count << 1);
#endif
          total_mapping_size += count * sizeof(mapping_node_t);
          total_mapping_nodes += count;
          m1->count += count;
          if (free_flag) {
            free_mapping(m1);
          }
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
      assign_svalue_no_free(newnode->values + 1, elt2->values + 1);
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

static void unique_add_to_mapping(mapping_t *m1, mapping_t *m2, int free_flag) {
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
            break;
          }
        } while ((elt1 = elt1->next));
        if (elt1) {
          continue;
        }
      } else if (!(--m1->unfilled)) {
        if (growMap(m1)) {
          a1 = m1->table;
          if (oi & ++mask) {
            n = a1[i |= mask];
          }
          mask <<= 1;
          mask--;
        } else {
          ++m1->unfilled;
          count -= MAP_COUNT(m1);
#ifdef PACKAGE_MUDLIB_STATS
          add_array_size(&m1->stats, count << 1);
#endif
          total_mapping_size += count * sizeof(mapping_node_t);
          total_mapping_nodes += count;
          m1->count += count;
          if (free_flag) {
            free_mapping(m1);
          }

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
      assign_svalue_no_free(newnode->values + 1, elt2->values + 1);
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

void absorb_mapping(mapping_t *m1, mapping_t *m2) {
  if (MAP_COUNT(m2)) {
    if (m1 != m2) {
      add_to_mapping(m1, m2, 0);
    }
  }
}

/*
   add_mapping: returns a new mapping that contains everything
   in two old mappings.  (uses hash table)
*/

mapping_t *add_mapping(mapping_t *m1, mapping_t *m2) {
  mapping_t *newmap;

  debug(mapping, "mapping.c: add_mapping begin: %p, %p", (void *)m1, (void *)m2);
  if (MAP_COUNT(m1) >= MAP_COUNT(m2)) {
    if (MAP_COUNT(m2)) {
      add_to_mapping(newmap = copyMapping(m1), m2, 1);
      return newmap;
    } else {
      return copyMapping(m1);
    }
  } else if (MAP_COUNT(m1)) {
    unique_add_to_mapping(newmap = copyMapping(m2), m1, 1);
    return newmap;
  } else {
    return copyMapping(m2);
  }
  debug(mapping, ("mapping.c: add_mapping end\n"));
}

/*
   map_mapping: A lot of the efuns that work on arrays, such as
   filter_array(), should also work on mappings.
*/

#ifdef F_MAP
void map_mapping(svalue_t *arg, int num_arg) {
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
  debug(mapping, ("mapping.c: map_mapping\n"));
  do {
    for (elt = a[j]; elt; elt = elt->next) {
      push_svalue(elt->values);
      push_svalue(elt->values + 1);
      ret = call_efun_callback(&ftc, 2);
      if (ret) {
        assign_svalue(elt->values + 1, ret);
      } else {
        break;
      }
    }
  } while (j--);

  pop_n_elems(num_arg - 1);
}
#endif

#ifdef F_FILTER
void filter_mapping(svalue_t *arg, int num_arg) {
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
  debug(mapping, ("mapping.c: filter_mapping\n"));
  do {
    for (elt = a[j]; elt; elt = elt->next) {
      push_svalue(elt->values);
      push_svalue(elt->values + 1);
      ret = call_efun_callback(&ftc, 2);
      if (!ret) {
        break;
      } else if (ret->type != T_NUMBER || ret->u.number) {
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
        assign_svalue_no_free(newnode->values + 1, elt->values + 1);
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

mapping_t *compose_mapping(mapping_t *m1, mapping_t *m2, unsigned short flag) {
  mapping_node_t *elt, *elt2, **a, **b = m2->table, **prev;
  unsigned int j = m1->table_size, deleted = 0;
  unsigned int mask = m2->table_size;
  svalue_t *sv;

  debug(mapping, ("mapping.c: compose_mapping\n"));
  if (flag) {
    m1 = copyMapping(m1);
  }
  a = m1->table;

  do {
    if ((elt = *(prev = a))) {
      do {
        sv = elt->values + 1;
        if ((elt2 = b[svalue_to_int(sv) & mask])) {
          do {
            if (msameval(sv, elt2->values)) {
              if (sv != elt2->values + 1) { /* if m1 == m2 */
                assign_svalue(sv, elt2->values + 1);
              }
              break;
            }
          } while ((elt2 = elt2->next));
        }
        if (!elt2) {
          if (!(*prev = elt->next) && !(*a)) {
            m1->unfilled++;
          }
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

array_t *mapping_indices(mapping_t *m) {
  array_t *v;
  int j = m->table_size;
  mapping_node_t *elt, **a = m->table;
  svalue_t *sv;

  debug(mapping, "mapping_indices: size = %d\n", MAP_COUNT(m));

  v = allocate_empty_array(MAP_COUNT(m));
  sv = v->item;
  do {
    for (elt = a[j]; elt; elt = elt->next) {
      assign_svalue_no_free(sv++, elt->values);
    }
  } while (j--);
  return v;
}

/* mapping_values */

array_t *mapping_values(mapping_t *m) {
  array_t *v;
  int j = m->table_size;
  mapping_node_t *elt, **a = m->table;
  svalue_t *sv;

  debug(mapping, "mapping_values: size = %d\n", MAP_COUNT(m));

  v = allocate_empty_array(MAP_COUNT(m));
  sv = v->item;
  do {
    for (elt = a[j]; elt; elt = elt->next) {
      assign_svalue_no_free(sv++, elt->values + 1);
    }
  } while (j--);
  return v;
}

/* functions for building mappings */

static svalue_t *insert_in_mapping(mapping_t *m, const char *key) {
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

void add_mapping_pair(mapping_t *m, const char *key, long value) {
  svalue_t *s;

  s = insert_in_mapping(m, key);
  s->type = T_NUMBER;
  s->subtype = 0;
  s->u.number = value;
}

void add_mapping_string(mapping_t *m, const char *key, const char *value) {
  svalue_t *s;

  s = insert_in_mapping(m, key);
  s->type = T_STRING;
  s->subtype = STRING_SHARED;
  s->u.string = make_shared_string(value);
}

void add_mapping_malloced_string(mapping_t *m, const char *key, char *value) {
  svalue_t *s;

  s = insert_in_mapping(m, key);
  s->type = T_STRING;
  s->subtype = STRING_MALLOC;
  s->u.string = value;
}

void add_mapping_object(mapping_t *m, const char *key, object_t *value) {
  svalue_t *s;

  s = insert_in_mapping(m, key);
  s->type = T_OBJECT;
  s->subtype = 0;
  s->u.ob = value;
  add_ref(value, "add_mapping_object");
}

void add_mapping_array(mapping_t *m, const char *key, array_t *value) {
  svalue_t *s;

  s = insert_in_mapping(m, key);
  s->type = T_ARRAY;
  s->subtype = 0;
  s->u.arr = value;
  value->ref++;
}

void add_mapping_shared_string(mapping_t *m, const char *key, char *value) {
  svalue_t *s;

  s = insert_in_mapping(m, key);
  s->type = T_STRING;
  s->subtype = STRING_SHARED;
  s->u.string = ref_string(value);
}
