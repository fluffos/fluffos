#ifndef ARRAY_H
#define ARRAY_H

#include "array.h"

/*
 * array.c
 */
extern struct vector null_vector;
extern int num_arrays;
extern int total_array_size;

int sameval PROT((struct svalue *, struct svalue *));
struct vector *null_array PROT((void));
struct vector *allocate_array PROT((int));
void free_vector PROT((struct vector *));
struct vector *add_array PROT((struct vector *, struct vector *));
struct vector *subtract_array PROT((struct vector *, struct vector *));
struct vector *slice_array PROT((struct vector *, int, int));
struct vector *explode_string PROT((char *, char *));
char *implode_string PROT((struct vector *, char *));
struct vector *users PROT((void));
struct vector *commands PROT((struct object *));
struct vector *deep_inherit_list PROT((struct object *));
struct vector *inherit_list PROT((struct object *));
struct vector *children PROT((char *));
struct vector *livings PROT((void));
struct vector *objects PROT((struct funp *));
struct vector *all_inventory PROT((struct object *, int));
struct vector *deep_inventory PROT((struct object *, int));
struct vector *filter PROT((struct vector *, struct funp *, struct svalue *));
struct vector *builtin_sort_array PROT((struct vector *, int));
struct vector *fp_sort_array PROT((struct vector *, struct funp *));
struct vector *sort_array PROT((struct vector *, char *, struct object *));
struct vector *make_unique PROT((struct vector *, char *, struct funp *, struct svalue *));
void map_array PROT((struct svalue *arg, int num_arg));
struct vector *intersect_array PROT((struct vector *, struct vector *));
struct vector *match_regexp PROT((struct vector *, char *));

#define ALLOC_VECTOR(nelem) \
    (struct vector *)DXALLOC(sizeof (struct vector) + \
	  sizeof(struct svalue) * (nelem - 1), 121, "ALLOC_VECTOR")

#endif
