/*
 * file.cc
 *
 *  Created on: Nov 18, 2014
 *      Author: sunyc
 */

#include <cstring>
#include <cstdlib>
#include <cctype>

/*
 * On Windows a backslash separates path components just like '/', and a
 * "C:" prefix makes a path absolute -- so a mudlib-supplied "..\\.." or
 * "C:\\" escapes the mudlib directory unless both are treated as such here.
 */
static inline bool is_path_separator(char c) {
#ifdef _WIN32
  return c == '/' || c == '\\';
#else
  return c == '/';
#endif
}

/*
 * Check that it is an legal path. No '..' are allowed.
 */
int legal_path(const char* path) {
  const char* p;

  if (path == nullptr) {
    return 0;
  }
  if (is_path_separator(path[0])) {
    return 0;
  }
#ifdef _WIN32
  if (isalpha(static_cast<unsigned char>(path[0])) && path[1] == ':') {
    return 0; /* drive-qualified, i.e. absolute */
  }
#endif

  /* Reject any `.' or `..' component; a single trailing `.' is allowed. */
  p = path;
  while (true) { /* Zak, 930530 - do better checking */
    if (p[0] == '.') {
      if (p[1] == '\0') { /* trailing `.' ok */
        break;
      }
      const char* q = (p[1] == '.') ? p + 1 : p; /* check for `..' or `../' */
      if (is_path_separator(q[1]) || q[1] == '\0') {
        return 0; /* check for `./', `..', or `../' */
      }
    }
    /* search next component */
    while (*p != '\0' && !is_path_separator(*p)) {
      p++;
    }
    if (*p == '\0') {
      break;
    }
    p++; /* step over the separator */
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
