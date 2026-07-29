#ifndef ARRAY_H
#define ARRAY_H

#include "vm/internal/base/svalue.h"

#ifdef PACKAGE_MUDLIB_STATS
#include "packages/mudlib_stats/mudlib_stats.h"
#endif

/*
 * The header and the element storage are SEPARATE allocations, and `item'
 * points at `capacity' element slots.
 *
 * This is the same split mapping_t has with its bucket table, and it exists
 * for the same reason: the header's address is stable across a resize.  An
 * array variable holds a pointer to this header, so with the elements inline
 * (the old `svalue_t item[1]' layout) growing an array necessarily moved it,
 * and every other holder of that array would have had to be found and told
 * the new address -- impossible, since holders are scattered across object
 * variable blocks, mappings, the VM stack and C++ containers.  With the
 * indirection the header never moves, so an array can grow in place and every
 * holder observes it, exactly as a mapping does when a key is added.
 *
 * `capacity' may exceed `size': callers that over-allocate and then shrink
 * (see fix_array) keep the slack rather than paying for a second realloc,
 * and growth into existing slack does not move the element block at all.
 * Memory accounting (total_array_size, checkmemory.cc) counts CAPACITY,
 * since that is what is allocated; mudlib stats count size, which is what
 * LPC sees.
 */
struct array_t {
  uint32_t ref;
#ifdef DEBUGMALLOC_EXTENSIONS
  int extra_ref;
#endif
  int size;
  int capacity; /* allocated element slots; always >= size */
#ifdef PACKAGE_MUDLIB_STATS
  statgroup_t stats; /* creator of the array */
#endif
  svalue_t* item; /* `capacity' slots, separately allocated */
};

extern array_t the_null_array;

/*
 * array.c
 */

int sameval(svalue_t*, svalue_t*);
array_t* allocate_array2(int, svalue_t*);
array_t* allocate_array(int);
array_t* allocate_empty_array(int);
void free_array(array_t*);
void free_empty_array(array_t*);
array_t* add_array(array_t*, array_t*);
void implode_array(funptr_t*, array_t*, svalue_t*, int);
array_t* subtract_array(array_t*, array_t*);
array_t* slice_array(array_t*, int, int);
array_t* explode_string(const char* str, int slen, const char* del, int dellen, bool reversible);
char* implode_string(array_t*, const char*, int);
array_t* commands(struct object_t*);
void filter_array(svalue_t*, int);
void filter_string(svalue_t*, int);
array_t* deep_inherit_list(struct object_t*);
array_t* inherit_list(struct object_t*);
array_t* children(const char*);
array_t* livings(void);
array_t* objects(funptr_t*);
array_t* all_inventory(struct object_t*, int);
array_t* deep_inventory(struct object_t*, int, funptr_t*);
array_t* deep_inventory_array(array_t*, int, funptr_t*);
array_t* filter(array_t*, funptr_t*, svalue_t*);
array_t* builtin_sort_array(array_t*, int);
array_t* fp_sort_array(array_t*, funptr_t*);
array_t* sort_array(array_t*, char*, struct object_t*);
array_t* make_unique(array_t*, char*, funptr_t*, svalue_t*);
void map_string(svalue_t* arg, int num_arg);
void map_array(svalue_t* arg, int num_arg);
array_t* intersect_array(array_t*, array_t*);
array_t* reg_assoc(svalue_t*, array_t*, array_t*, svalue_t*);
void dealloc_array(array_t*);
array_t* union_array(array_t*, array_t*);
array_t* copy_array(array_t* p);
/* Set ->size to n, making ->capacity exactly n (both directions). */
array_t* resize_array(array_t* p, unsigned int n);
/* Ensure ->capacity >= n, growing geometrically; ->size is untouched. */
void array_reserve(array_t* p, unsigned int n);

/* Bytes of element storage for `nelem' slots. */
#define ARRAY_ITEMS_SIZE(nelem) (sizeof(svalue_t) * (nelem))

/* The header alone.  Element storage is allocated separately and hung off
 * ->item; ALLOC_ARRAY() below does both and is what callers want. */
#define ALLOC_ARRAY_HDR() (array_t*)DCALLOC(sizeof(array_t), 1, TAG_ARRAY, "ALLOC_ARRAY_HDR")

#define ALLOC_ARRAY_ITEMS(nelem) \
  (svalue_t*)DMALLOC(ARRAY_ITEMS_SIZE(nelem), TAG_ARRAY_ITEMS, "ALLOC_ARRAY_ITEMS")
#define RESIZE_ARRAY_ITEMS(items, nelem)                                   \
  (svalue_t*)DREALLOC(items, ARRAY_ITEMS_SIZE(nelem), TAG_ARRAY_ITEMS,     \
                      "RESIZE_ARRAY_ITEMS")

/* Header + `nelem' element slots, capacity set, size left to the caller.
 * Returns nullptr if either allocation fails. */
array_t* alloc_array_block(unsigned int nelem);
#define ALLOC_ARRAY(nelem) alloc_array_block(nelem)

#endif
