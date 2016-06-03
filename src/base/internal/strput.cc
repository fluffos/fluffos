/*
 * strput.cc
 */

#include <cstdio>

char *strput(char *x, const char *limit, const char *y) {
  while ((*x++ = *y++)) {
    if (x == limit) {
      *(x - 1) = 0;
      break;
    }
  }
  return x - 1;
}

char *strput_int(char *x, const char *limit, int num) {
  char buf[20];
  snprintf(buf, 20, "%d", num);
  return strput(x, limit, buf);
}
