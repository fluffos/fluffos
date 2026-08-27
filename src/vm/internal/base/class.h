#ifndef CLASS_H
#define CLASS_H

#include "vm/internal/base/svalue.h" /* lpc_type_t */

struct class_def_t {
  unsigned short classname;
  unsigned short type;
  unsigned short size;
  unsigned short index;
};

struct class_member_entry_t {
  unsigned short membername;
  lpc_type_t type;
};

void dealloc_class(struct array_t*);
void free_class(struct array_t*);
struct array_t* allocate_class(class_def_t*, int);
struct array_t* allocate_class_by_size(int);
struct array_t* allocate_empty_class_by_size(int);

#endif
