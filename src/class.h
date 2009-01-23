#ifndef CLASS_H
#define CLASS_H

/* It is usually better to include "lpc_incl.h" instead of including this
   directly */

#ifdef CLASS_STATS
extern int num_classes;
extern int total_class_size;
#endif


void dealloc_class (array_t *);
void free_class (array_t *);
array_t *allocate_class (class_def_t *, int);
array_t *allocate_class_by_size (int);
array_t *allocate_empty_class_by_size (int);

#endif
