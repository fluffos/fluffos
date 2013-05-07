/*
  Author: Zoilder (RL-Mud)
  Date: 03/05/2013
  Description: Package that remove leading / trailing whitespaces and other type of characters (\n \t ...) in a string.
  Functions:
    - trim: Remove leading and trailing whitespaces (and others).
      Example:
        - "    my test   " : "my test"
    - ltrim: Remove leading whitespaces (and others).
      Example:
        - "    my test   " : "my test   "
    - rtrim: Remove trailing whitespaces (and others).
      Example:
        - "    my test   " : "    my test"

  Characters that remove by default (it can be configured as parameter 2 in efuns):
    ' '  (0x20)	space (SPC)
    '\t' (0x09)	horizontal tab (TAB)
    '\n' (0x0a)	newline (LF)
    '\r' (0x0d)	carriage return (CR)
*/
#include "../lpc_incl.h"

/*
  Function that removes leading whitespaces / character list.
*/
char *ltrim(const char *str, const char *charlist){
  const char *ptr_start, *ptr_end;
  char *ret;
  int size;

  // Is str is null or empty, we return empty string.
  if(!str || str[0] == '\0'){
    ret = new_string(0, "ltrim:empty");
    ret[0] = '\0';
    return ret;
  }

  // Start and end string.
  ptr_start = str;
  ptr_end   = &str[strlen(str) - 1];

  // Search final text skipping leading whitespaces / character list.
  while(ptr_start <= ptr_end && (!charlist && isspace(*ptr_start) || charlist && strchr(charlist, *ptr_start)))
    ptr_start++;

  // There aren't any text.
  if(ptr_start > ptr_end){
    ret = new_string(0, "ltrim:empty");
    ret[0] = '\0';
    return ret;
  }

  // We prepare a substring without whitespaces / character list and return it.
  size = ptr_end - ptr_start + 1;
  ret = new_string(size, "ltrim");
  strncpy(ret, ptr_start, size);
  ret[size] = '\0';
  return ret;
}

/*
  Function that removes trailing whitespaces / character list.
*/
char *rtrim(const char *str, const char *charlist){
  const char *ptr_end;
  char *ret;
  int size;

  // Is str is null or empty, we return empty string.
  if(!str || str[0] == '\0'){
    ret = new_string(0, "rtrim:empty");
    ret[0] = '\0';
    return ret;
  }

  // End of string.
  ptr_end   = &str[strlen(str) - 1];

  // Search final text skipping trailing whitespaces / character list.
  while(ptr_end >= str && (!charlist && isspace(*ptr_end) || charlist && strchr(charlist, *ptr_end)))
    ptr_end--;

  // There aren't any text.
  if(ptr_end < str){
    ret = new_string(0, "rtrim:empty");
    ret[0] = '\0';
    return ret;
  }

  // We prepare a substring without whitespaces / character list and return it.
  size = ptr_end - str + 1;
  ret = new_string(size, "rtrim");
  strncpy(ret, str, size);
  ret[size] = '\0';
  return ret;
}

#ifdef F_TRIM
void
f_trim(void)
{
  const char *str, *charlist = NULL;

  // If use 2 arguments, we get characters to remove and positioning in previous parameter.
  if(st_num_arg == 2){
    charlist = string_copy(sp->u.string, "f_trim");
    pop_stack();
  }

  // Unlink for modify string without modify origin string.
  unlink_string_svalue(sp);

  // Remove blank spaces.
  str = ltrim(rtrim(sp->u.string, charlist), charlist);

  // Update string.
  free_string_svalue(sp);
  put_malloced_string(str);
}
#endif

#ifdef F_LTRIM
void
f_ltrim(void)
{
  const char *str, *charlist = NULL;

  // If use 2 arguments, we get characters to remove and positioning in previous parameter.
  if(st_num_arg == 2){
    charlist = string_copy(sp->u.string, "f_ltrim");
    pop_stack();
  }

  // Unlink for modify string withoud modify origin string.
  unlink_string_svalue(sp);

  // Remove blank spaces.
  str = ltrim(sp->u.string, charlist);

  // Update string.
  free_string_svalue(sp);
  put_malloced_string(str);
}
#endif

#ifdef F_RTRIM
void
f_rtrim(void)
{
  const char *str, *charlist = NULL;

  // If use 2 arguments, we get characters to remove and positioning in previous parameter.
  if(st_num_arg == 2){
    charlist = string_copy(sp->u.string, "f_rtrim");
    pop_stack();
  }

  // Unlink for modify string withoud modify origin string.
  unlink_string_svalue(sp);

  // Remove blank spaces.
  str = rtrim(sp->u.string, charlist);

  // Update string.
  free_string_svalue(sp);
  put_malloced_string(str);
}
#endif
 