#include "base/std.h"

#include "vm/internal/base/machine.h"

/*
 * A class is an array_t, so its element storage is a separate allocation too
 * -- see the comment on array_t in array.h.  Classes are fixed size and never
 * grow, so capacity is just the declared size, with a floor of one slot: a
 * zero-size class still needs a valid ->item, which the old inline layout
 * provided for free.
 */
static array_t* alloc_class_block(int size) {
  int const slots = size > 0 ? size : 1;
  array_t* p = reinterpret_cast<array_t*>(DMALLOC(sizeof(array_t), TAG_CLASS, "allocate_class"));

  p->item = reinterpret_cast<svalue_t*>(
      DMALLOC(ARRAY_ITEMS_SIZE(slots), TAG_CLASS_ITEMS, "allocate_class"));
  p->capacity = slots;
  p->item_locks = 0; /* DMALLOC does not zero, unlike ALLOC_ARRAY_HDR */
  p->ref = 1;
  p->size = size;

  num_classes++;
  total_class_size += sizeof(array_t) + ARRAY_ITEMS_SIZE(slots);

  return p;
}

void dealloc_class(array_t* p) {
  int i;

  num_classes--;
  total_class_size -= sizeof(array_t) + ARRAY_ITEMS_SIZE(p->capacity);

  for (i = p->size; i--;) {
    free_svalue(&p->item[i], "dealloc_class");
  }
  FREE((char*)p->item);
  FREE((char*)p);
}

void free_class(array_t* p) {
  if (--(p->ref) > 0) {
    return;
  }

  dealloc_class(p);
}

array_t* allocate_class(class_def_t* cld, int has_values) {
  array_t* p = alloc_class_block(cld->size);
  int n = cld->size;

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

  p = alloc_class_block(size);

  while (size--) {
    p->item[size] = const0u;
  }

  return p;
}

array_t* allocate_empty_class_by_size(int size) {
  if (size < 0 || size > CONFIG_INT(__MAX_ARRAY_SIZE__)) {
    error("Illegal class size.\n");
  }

  return alloc_class_block(size);
}
