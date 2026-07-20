#include "base/std.h"

#include "vm/internal/base/machine.h"

void dealloc_class(array_t* p) {
  int i;

  num_classes--;
  total_class_size -= sizeof(array_t) + sizeof(svalue_t) * (p->size - 1);

  for (i = p->size; i--;) {
    free_svalue(&p->item[i], "dealloc_class");
  }
  FREE((char*)p);
}

void free_class(array_t* p) {
  if (--(p->ref) > 0) {
    return;
  }

  dealloc_class(p);
}

array_t* allocate_class(class_def_t* cld, int has_values) {
  array_t* p;
  int n = cld->size;
  if (!n) {
    n++;
  }

  num_classes++;
  total_class_size += sizeof(array_t) + sizeof(svalue_t) * (n - 1);

  p = reinterpret_cast<array_t*>(
      DMALLOC(sizeof(array_t) + sizeof(svalue_t) * (n - 1), TAG_CLASS, "allocate_class"));
  n = cld->size;
  p->ref = 1;
  p->size = n;
  if (has_values) {
    while (n--) {
      p->item[n] = *sp--;
    }
  } else {
    while (n--) {
      p->item[n] = const0u;
    }
  }
  return p;
}

array_t* allocate_class_by_size(int size) {
  array_t* p;

  // Unlike allocate_array()/allocate_empty_array(), this took `size`
  // straight into a DMALLOC() byte-count computation with no validation at
  // all -- a garbage or out-of-range size (e.g. uninitialized restore-scratch
  // state reaching restore_class()) produced a garbage byte count and
  // crashed the debug allocator's own "illegal size" check instead of
  // failing cleanly. Match allocate_empty_array()'s bound.
  if (size < 0 || size > CONFIG_INT(__MAX_ARRAY_SIZE__)) {
    error("Illegal class size.\n");
  }

  num_classes++;
  total_class_size += sizeof(array_t) + sizeof(svalue_t) * (size - 1);

  p = reinterpret_cast<array_t*>(
      DMALLOC(sizeof(array_t) + sizeof(svalue_t) * (size - 1), TAG_CLASS, "allocate_class"));
  p->ref = 1;
  p->size = size;

  while (size--) {
    p->item[size] = const0u;
  }

  return p;
}

array_t* allocate_empty_class_by_size(int size) {
  array_t* p;

  if (size < 0 || size > CONFIG_INT(__MAX_ARRAY_SIZE__)) {
    error("Illegal class size.\n");
  }

  num_classes++;
  total_class_size += sizeof(array_t) + sizeof(svalue_t) * (size - 1);

  p = reinterpret_cast<array_t*>(
      DMALLOC(sizeof(array_t) + sizeof(svalue_t) * (size - 1), TAG_CLASS, "allocate_class"));
  p->ref = 1;
  p->size = size;

  return p;
}