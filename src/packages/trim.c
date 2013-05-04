/*
  Author: Zoilder (RL-Mud)
  Date: 03/05/2013
  Description: Package that remove leading / trailing whitespaces in a string.
  Functions:
    - trim: Remove leading and trailing whitespaces.
      Example:
        - "    my test   " : "my test"
    - ltrim: Remove leading whitespaces.
      Example:
        - "    my test   " : "my test   "
    - rtrim: Remove trailing whitespaces.
      Example:
        - "    my test   " : "    my test"
*/
#include "../lpc_incl.h"

/*
  Function that removes leading whitespaces.
*/
char *ltrim(const char *str){
  const char *ptr_start, *ptr_end;
  char *ret;
  int size;

  // Is str is null or empty, we return empty string.
  if(!str || str[0] == '\0'){
    ret = new_string(1, "trim:empty");
    ret[0] = '\0';
    return ret;
  }

  // Start and end string.
  ptr_start = str;
  ptr_end   = ptr_start + strlen(ptr_start) - 1;

  // Search initial text skipping leading whitespaces.
  while(isspace(*ptr_start))
    ptr_start++;

  // There aren't any text.
  if(ptr_start > ptr_end){
    ret = new_string(1, "trim:empty");
    ret[0] = '\0';
    return ret;
  }

  // We prepare a substring without whitespaces and return it.
  size = ptr_end - ptr_start + 1;
  ret = new_string(size, "f_trim");
  strncpy(ret, ptr_start, size);
  ret[size] = '\0';
  return ret;
}

/*
  Function that removes trailing whitespaces.
*/
char *rtrim(const char *str){
  const char *ptr_end;
  char *ret;
  int size;

  // Is str is null or empty, we return empty string.
  if(!str || str[0] == '\0'){
    ret = new_string(1, "trim:empty");
    ret[0] = '\0';
    return ret;
  }

  // End of string.
  ptr_end   = str + strlen(str) - 1;

  // Search final text skipping trailing whitespaces.
  while(isspace(*ptr_end) && ptr_end > str)
    ptr_end--;

  // There aren't any text.
  if(ptr_end == str){
    ret = new_string(1, "trim:empty");
    ret[0] = '\0';
    return ret;
  }

  // We prepare a substring without whitespaces and return it.
  size = ptr_end - str + 1;
  ret = new_string(size, "f_trim");
  strncpy(ret, str, size);
  ret[size] = '\0';
  return ret;
}

#ifdef F_TRIM
void
f_trim(void)
{
  const char *str;

  // If parameter isn't string, we return "".
  if(sp->type != T_STRING){
    put_constant_string("");
    return;
  }
  
  // Parameter is string.
    // Unlink for modify string withoud modify origin string.
  unlink_string_svalue(sp);

  // Remove blank spaces.
  str = ltrim(rtrim(sp->u.string));

  // Update string.
  put_malloced_string(str);
}
#endif

#ifdef F_LTRIM
void
f_ltrim(void)
{
  const char *str;

  // If parameter isn't string, we return "".
  if(sp->type != T_STRING){
    put_constant_string("");
    return;
  }
  
  // Parameter is string.
    // Unlink for modify string withoud modify origin string.
  unlink_string_svalue(sp);

  // Remove blank spaces.
  str = ltrim(sp->u.string);

  // Update string.
  put_malloced_string(str);
}
#endif

#ifdef F_RTRIM
void
f_rtrim(void)
{
  const char *str;

  // If parameter isn't string, we return "".
  if(sp->type != T_STRING){
    put_constant_string("");
    return;
  }
  
  // Parameter is string.
    // Unlink for modify string withoud modify origin string.
  unlink_string_svalue(sp);

  // Remove blank spaces.
  str = rtrim(sp->u.string);

  // Update string.
  put_malloced_string(str);
}
#endif