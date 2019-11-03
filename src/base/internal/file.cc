/*
 * file.cc
 *
 *  Created on: Nov 18, 2014
 *      Author: sunyc
 */

#include "base/internal/file.h"

#include <cstring>
#include <cstdlib>

/*
 * Check that it is an legal path. No '..' are allowed.
 */
/**
 * @brief Check that it is an legal path. No '..' are allowed.
 *
 * @param path
 *
 * @return 1 if true, 0 otherwise
 */
int legal_path(const std::string path) {
    size_t i = 0;
    size_t s = path.size();

  if (s == 0) {
    return 0;
  }
  if (path[0] == '/') {
    return 0;
  }
  /*
   * disallowing # seems the easiest way to solve a bug involving loading
   * files containing that character
   */
  if (path.find('#') != std::string::npos) {
    return 0;
  }
  while (i < s) {
    if (path[i] == '.') {
      if ((i + 1) == s) { /* trailing `.' ok */
        break;
      }
      if (path[i+1] == '.') { /* check for `..' or `../' */
        i++;
      }
      if (((i + 1) == s) || (path[i + 1] == '/')) {
        return 0; /* check for `./', `..', or `../' */
      }
    }
    i = path.find("/.", i); /* search next component */
    if (i != std::string::npos) {
      i++; /* step over `/' */
    }
  }
  return 1;
} /* legal_path() */
