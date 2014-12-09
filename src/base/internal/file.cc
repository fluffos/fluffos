/*
 * file.cc
 *
 *  Created on: Nov 18, 2014
 *      Author: sunyc
 */

#include <cstring>
#include <cstdlib>

/*
 * Check that it is an legal path. No '..' are allowed.
 */
int legal_path(const char *path) {
  const char *p;

  if (path == nullptr) {
    return 0;
  }
  if (path[0] == '/') {
    return 0;
  }
  /*
   * disallowing # seems the easiest way to solve a bug involving loading
   * files containing that character
   */
  if (strchr(path, '#')) {
    return 0;
  }
  p = path;
  while (p) { /* Zak, 930530 - do better checking */
    if (p[0] == '.') {
      if (p[1] == '\0') { /* trailing `.' ok */
        break;
      }
      if (p[1] == '.') { /* check for `..' or `../' */
        p++;
      }
      if (p[1] == '/' || p[1] == '\0') {
        return 0; /* check for `./', `..', or `../' */
      }
    }
    p = const_cast<char *>(strstr(p, "/.")); /* search next component */
    if (p) {
      p++; /* step over `/' */
    }
  }
  return 1;
} /* legal_path() */
