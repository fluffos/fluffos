#ifndef ARRAY_H
#define ARRAY_H

/* It is usually better to include "lpc_incl.h" instead of including this
   directly */

typedef struct array_s {
    unsigned short ref;
#ifdef DEBUG
    int extra_ref;
#endif
    unsigned short size;
#ifdef PACKAGE_MUDLIB_STATS
    statgroup_t stats;          /* creator of the array */
#endif
    svalue_t item[1];
} array_t;

extern array_t the_null_array;

/*
 * array.c
 */
#ifdef ARRAY_STATS
extern int num_arrays;
extern int total_array_size;
#endif

int sameval (svalue_t *, svalue_t *);
array_t *allocate_array2 (int, svalue_t *);
array_t *allocate_array (int);
array_t *allocate_empty_array (int);
void free_array (array_t *);
void free_empty_array (array_t *);
array_t *add_array (array_t *, array_t *);
void implode_array (funptr_t *, array_t *, svalue_t *, int);
array_t *subtract_array (array_t *, array_t *);
array_t *slice_array (array_t *, int, int);
array_t *explode_string (const char *, int, const char *, int);
char *implode_string (array_t *, const char *, int);
array_t *users (void);
array_t *commands (object_t *);
void filter_array (svalue_t *, int);
void filter_string (svalue_t *, int);
array_t *deep_inherit_list (object_t *);
array_t *inherit_list (object_t *);
array_t *children (const char *);
array_t *livings (void);
array_t *objects (funptr_t *);
array_t *all_inventory (object_t *, int);
array_t *deep_inventory (object_t *, int);
array_t *filter (array_t *, funptr_t *, svalue_t *);
array_t *builtin_sort_array (array_t *, int);
array_t *fp_sort_array (array_t *, funptr_t *);
array_t *sort_array (array_t *, char *, object_t *);
array_t *make_unique (array_t *, char *, funptr_t *, svalue_t *);
void map_string (svalue_t *arg, int num_arg);
void map_array (svalue_t *arg, int num_arg);
array_t *intersect_array (array_t *, array_t *);
int match_single_regexp (const char *, const char *);
array_t *match_regexp (array_t *, const char *, int);
array_t *reg_assoc (const char *, array_t *, array_t *, svalue_t *);
void dealloc_array (array_t *);
array_t *union_array (array_t *, array_t *);
array_t *copy_array (array_t * p);
array_t *resize_array (array_t * p, unsigned short n);

#define ALLOC_ARRAY(nelem) \
    (array_t *)DXALLOC(sizeof (array_t) + \
          sizeof(svalue_t) * (nelem - 1), TAG_ARRAY, "ALLOC_ARRAY")
#define RESIZE_ARRAY(vec, nelem) \
    (array_t *)DREALLOC(vec, sizeof (array_t) + \
          sizeof(svalue_t) * (nelem - 1), TAG_ARRAY, "RESIZE_ARRAY")

#endif
