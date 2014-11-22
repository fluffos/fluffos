/*
 * strput.cc
 */

#include <cstdio>

char *strput(char *x, char *limit, const char *y) {
  while ((*x++ = *y++)) {
    if (x == limit) {
      *(x - 1) = 0;
      break;
    }
  }
  return x - 1;
}

char *strput_int(char *x, char *limit, int num) {
  char buf[20];
  sprintf(buf, "%d", num);
  return strput(x, limit, buf);
}
