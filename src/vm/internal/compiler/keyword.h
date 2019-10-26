/*
 * keyword.h
 *
 *  Created on: Nov 18, 2014
 *      Author: sunyc
 */

#ifndef SRC_VM_INTERNAL_COMPILER_KEYWORD_H_
#define SRC_VM_INTERNAL_COMPILER_KEYWORD_H_

struct keyword_t {
  const char *word;
  unsigned short token;     /* flags here too */
  unsigned short sem_value; /* semantic value for predefined tokens */
  struct ident_hash_elem_t *next;
  /* the fields above must correspond to struct ident_hash_elem */
  short min_args;           /* Minimum number of arguments. */
  short max_args;           /* Maximum number of arguments. */
  unsigned short ret_type;  /* The return type used by the compiler. */
  unsigned short arg_type1; /* Type of argument 1 */
  unsigned short arg_type2; /* Type of argument 2 */
  unsigned short arg_type3; /* Type of argument 3 */
  unsigned short arg_type4; /* Type of argument 4 */
  short arg_index;          /* Index pointing to where to find arg type */
  short Default;            /* an efun to use as default for last
                             * argument */
};

#endif /* SRC_VM_INTERNAL_COMPILER_KEYWORD_H_ */
