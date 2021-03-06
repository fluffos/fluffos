#ifndef ARRAY_H
#define ARRAY_H

#include "vm/internal/base/svalue.h"

#ifdef PACKAGE_MUDLIB_STATS
#include "packages/mudlib_stats/mudlib_stats.h"
#endif

struct array_t {
  uint32_t ref;
#ifdef DEBUGMALLOC_EXTENSIONS
  int extra_ref;
#endif
  int size;
#ifdef PACKAGE_MUDLIB_STATS
  statgroup_t stats; /* creator of the array */
#endif
  svalue_t item[1];
};

extern array_t the_null_array;

/*
 * array.c
 */

int sameval(svalue_t *, svalue_t *);
array_t *allocate_array2(int, svalue_t *);
array_t *allocate_array(int);
array_t *allocate_empty_array(int);
void free_array(array_t *);
void free_empty_array(array_t *);
array_t *add_array(array_t *, array_t *);
void implode_array(funptr_t *, array_t *, svalue_t *, int);
array_t *subtract_array(array_t *, array_t *);
array_t *slice_array(array_t *, int, int);
array_t *explode_string(const char *str, int slen, const char *del, int dellen, bool reversible);
char *implode_string(array_t *, const char *, int);
array_t *commands(struct object_t *);
void filter_array(svalue_t *, int);
void filter_string(svalue_t *, int);
array_t *deep_inherit_list(struct object_t *);
array_t *inherit_list(struct object_t *);
array_t *children(const char *);
array_t *livings(void);
array_t *objects(funptr_t *);
array_t *all_inventory(struct object_t *, int);
array_t *deep_inventory(struct object_t *, int, funptr_t *);
array_t *deep_inventory_array(array_t *, int, funptr_t *);
array_t *filter(array_t *, funptr_t *, svalue_t *);
array_t *builtin_sort_array(array_t *, int);
array_t *fp_sort_array(array_t *, funptr_t *);
array_t *sort_array(array_t *, char *, struct object_t *);
array_t *make_unique(array_t *, char *, funptr_t *, svalue_t *);
void map_string(svalue_t *arg, int num_arg);
void map_array(svalue_t *arg, int num_arg);
array_t *intersect_array(array_t *, array_t *);
array_t *reg_assoc(svalue_t *, array_t *, array_t *, svalue_t *);
void dealloc_array(array_t *);
array_t *union_array(array_t *, array_t *);
array_t *copy_array(array_t *p);
array_t *resize_array(array_t *p, unsigned int n);

#define ALLOC_ARRAY(nelem)                                                           \
  (array_t *)DCALLOC(sizeof(array_t) + sizeof(svalue_t) * (nelem - 1), 1, TAG_ARRAY, \
                     "ALLOC_"                                                        \
                     "ARRAY")
#define RESIZE_ARRAY(vec, nelem)                                                        \
  (array_t *)DREALLOC(vec, sizeof(array_t) + sizeof(svalue_t) * (nelem - 1), TAG_ARRAY, \
                      "RESIZE_ARRAY")

#endif
