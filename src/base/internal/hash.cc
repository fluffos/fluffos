#include "base/std.h"

#include "hash.h"

unsigned int whashstr(const char *s) {
  int i = 0;
  unsigned long h = 0;
  for (; *s && i++ < 100; ++s) {
    h = 37 * h + *s;
  }
  return h;
}
