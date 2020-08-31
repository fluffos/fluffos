#include "base/std.h"

#include "hash.h"

unsigned int whashstr(const char *s) {
  int i = 0;
  unsigned long _h = 0;
  for (; *s && i++ < 100; ++s) {
    _h = 37 * _h + *s;
  }
  return _h;
}
