#ifndef ARRAY_H
#define ARRAY_H

typedef struct array_s {
    unsigned short ref;
#ifdef DEBUG
    int extra_ref;
#endif
    unsigned short size;
#ifdef PACKAGE_MUDLIB_STATS
    statgroup_t stats;		/* creator of the array */
#endif
    svalue_t item[1];
} array_t;

/*
 * array.c
 */
extern int num_arrays;
extern int total_array_size;

int sameval PROT((svalue_t *, svalue_t *));
array_t *null_array PROT((void));
array_t *allocate_array PROT((int));
array_t *allocate_empty_array PROT((int));
void free_array PROT((array_t *));
void free_empty_array PROT((array_t *));
array_t *add_array PROT((array_t *, array_t *));
void implode_array PROT((funptr_t *, array_t *, svalue_t *, int));
array_t *subtract_array PROT((array_t *, array_t *));
array_t *slice_array PROT((array_t *, int, int));
array_t *explode_string PROT((char *, int, char *, int));
char *implode_string PROT((array_t *, char *, int));
array_t *users PROT((void));
array_t *commands PROT((object_t *));
void filter_array PROT((svalue_t *, int));
array_t *deep_inherit_list PROT((object_t *));
array_t *inherit_list PROT((object_t *));
array_t *children PROT((char *));
array_t *livings PROT((void));
array_t *objects PROT((funptr_t *));
array_t *all_inventory PROT((object_t *, int));
array_t *deep_inventory PROT((object_t *, int));
array_t *filter PROT((array_t *, funptr_t *, svalue_t *));
array_t *builtin_sort_array PROT((array_t *, int));
array_t *fp_sort_array PROT((array_t *, funptr_t *));
array_t *sort_array PROT((array_t *, char *, object_t *));
array_t *make_unique PROT((array_t *, char *, funptr_t *, svalue_t *));
void map_string PROT((svalue_t *arg, int num_arg));
void map_array PROT((svalue_t *arg, int num_arg));
array_t *intersect_array PROT((array_t *, array_t *));
array_t *match_regexp PROT((array_t *, char *, int));
array_t *reg_assoc PROT((char *, array_t *, array_t *, svalue_t *));
void dealloc_array PROT((array_t *));

#define ALLOC_ARRAY(nelem) \
    (array_t *)DXALLOC(sizeof (array_t) + \
	  sizeof(svalue_t) * (nelem - 1), TAG_ARRAY, "ALLOC_ARRAY")
#define RESIZE_ARRAY(vec, nelem) \
    (array_t *)DREALLOC(vec, sizeof (array_t) + \
	  sizeof(svalue_t) * (nelem - 1), TAG_ARRAY, "RESIZE_ARRAY")
#endif
