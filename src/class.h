#ifndef CLASS_H
#define CLASS_H

void dealloc_class PROT((array_t *));
void free_class PROT((array_t *));
array_t *allocate_class PROT((class_def_t *, int));
array_t *allocate_class_by_size PROT((int));

#endif
