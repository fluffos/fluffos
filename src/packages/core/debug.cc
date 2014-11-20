/*
 * debug.cc
 *
 *  Created on: Nov 16, 2014
 *      Author: sunyc
 */

#include "base/package_api.h"

mapping_t *debug_levels(void);

mapping_t *debug_levels() {
  int dl = debug_level;
  mapping_t *ret = allocate_mapping(10);
  unsigned int i;

  for (i = 0; i < sizeof_levels; i++) {
    add_mapping_pair(ret, levels[i].name, dl & levels[i].bit);
    dl &= ~levels[i].bit;
  }

  if (dl) {
    add_mapping_pair(ret, "unknown", dl);
  }

  return ret;
}
