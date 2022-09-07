/*
 * file.cc
 *
 *  Created on: Nov 18, 2014
 *      Author: sunyc
 */

#include <cstring>
#include <cstdlib>
#include <ctype.h>

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

  /*
   * disallow # if it is not indicating an OID
   */
  p = strchr(path, '#');
  while (p) {
    p++;
    if (p[0] == '\0') {
      break;
    }
    if (isdigit(p[0])) {
      continue;
    }
    return 0;
  }

  return 1;
} /* legal_path() */
