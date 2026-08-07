#ifndef LPC_SVALUE_H
#define LPC_SVALUE_H

/* It is usually better to include "lpc_incl.h" instead of including this
 directly */
#include "vm/internal/base/number.h"
#include <nlohmann/json_fwd.hpp>
using json = nlohmann::json;

typedef struct {
  uint32_t ref;
} refed_t;

typedef const char* LPC_STRING;
union u {
  LPC_INT number;
  LPC_FLOAT real;
  LPC_STRING string;

  refed_t* refed; /* any of the block below */

  struct buffer_t* buf;
  struct object_t* ob;
  struct array_t* arr;
  struct mapping_t* map;
  struct funptr_t* fp;

  struct svalue_t* lvalue;
  struct ref_t* ref;
  unsigned char* lvalue_byte;
  void (*error_handler)(void);

  struct promise_t* prom;
};

/*
 * The value stack element.
 * If it is a string, then the way that the string has been allocated
 * differently, which will affect how it should be freed.
 */
struct svalue_t {
  /* 32-bit: the 16 low type bits are fully allocated (see the T_* defines
   * below), so new value types must use bits at 0x10000 and above. Layout is
   * unchanged on 64-bit targets: 4 + 2 + 2 bytes padding + 8-byte union. */
  uint32_t type;
  unsigned short subtype;
  union u u;
};

struct ref_t {
  uint32_t ref;

  struct ref_t *next, *prev;
  struct control_stack_t* csp;
  svalue_t* lvalue;
  svalue_t sv;

  /* Set alongside lvalue == &global_lvalue_codepoint_sv (interpret.cc): this
   * ref's OWN owning string and EGC index, so a concurrently-armed string-char
   * lvalue elsewhere (another ref, or a plain s[i]) can't corrupt what this
   * ref reads/writes. The shared global is re-armed from these right before
   * each use (read via F_REF, write via F_REF_LVALUE); unused otherwise.
   * ref_t is raw-malloc'd (make_ref()), so these carry no implicit default --
   * make_ref() sets them, and they are only meaningful once the codepoint
   * arming site (F_NEXT_FOREACH) also sets lvalue to the sentinel above. */
  svalue_t* codepoint_owner;
  int32_t codepoint_index;
};

/* values for type field of svalue struct */
#define T_INVALID 0x0u
#define T_LVALUE 0x1u

#define T_NUMBER 0x2u
#define T_STRING 0x4u
#define T_REAL 0x80u

#define T_ARRAY 0x8u
#define T_OBJECT 0x10u
#define T_MAPPING 0x20u
#define T_FUNCTION 0x40u
#define T_BUFFER 0x100u
#define T_CLASS 0x200u

#define T_LVALUE_BYTE 0x400u /* byte-sized lvalue */
#define T_LVALUE_RANGE 0x800u
#define T_ERROR_HANDLER 0x1000u
#define T_FREED 0x2000u
#define T_REF 0x4000u
#define T_LVALUE_CODEPOINT 0x8000u /* UTF8 codepoint */

/* The 16 low bits are fully allocated; new value types start at 0x10000
 * (svalue_t::type is 32-bit). */
#define T_PROMISE 0x10000u

/*
 * Compile-time type words. These are a separate namespace from the runtime
 * T_* tags above: a base type (TYPE_* in compiler.h) or a class index, plus
 * the TYPE_MOD_* bits here and the DECL_* / LOCAL_MOD_* bits in program.h.
 *
 * lpc_type_t is 32 bits wide -- the low 16 are fully allocated, and
 * promise<T> (issue #1319) encodes its payload type in the same word as the
 * declaration's own array modifier.
 */
typedef uint32_t lpc_type_t;

#define TYPE_MOD_ARRAY 0x8000u /* Pointer to a basic type */
/* Note, the following restricts class_num to < 0x80 or 128, since the class
 * index shares the low byte with TYPE_MOD_CLASS. */
#define TYPE_MOD_CLASS 0x0080u /* a class */
#define CLASS_NUM_MASK 0x007fu

/*
 * promise<T>: TYPE_MOD_PROMISE says the declared value is a promise, and the
 * rest of the word describes its PAYLOAD (base type or class index).
 * TYPE_MOD_ARRAY keeps its exact ordinary meaning -- "this declaration is an
 * array" -- so `promise<int> *` (an array of promises) is
 * TYPE_MOD_PROMISE | TYPE_MOD_ARRAY | TYPE_NUMBER. The payload's own
 * array-ness is TYPE_MOD_PROMISE_VALUE_ARRAY, so `promise<int *>` is
 * TYPE_MOD_PROMISE | TYPE_MOD_PROMISE_VALUE_ARRAY | TYPE_NUMBER and the two
 * compose. Bare `promise` means promise<mixed>.
 *
 * These two bits sit in the gap between the basic type (bits 0-15) and the
 * declaration modifiers the parser parks in bits 21-30 (see rule_type() and
 * BASIC_TYPE_MASK in compiler.h).
 */
#define TYPE_MOD_PROMISE 0x10000u
#define TYPE_MOD_PROMISE_VALUE_ARRAY 0x20000u

#define T_REFED \
  (T_ARRAY | T_OBJECT | T_MAPPING | T_FUNCTION | T_BUFFER | T_CLASS | T_REF | T_PROMISE)
#define T_ANY (T_REFED | T_STRING | T_NUMBER | T_REAL)

/* values for subtype field of svalue struct */
#define STRING_COUNTED 0x1u /* has a length an ref count */
#define STRING_HASHED 0x2u  /* is in the shared string table */

#define STRING_MALLOC STRING_COUNTED
#define STRING_SHARED (STRING_COUNTED | STRING_HASHED)
#define STRING_CONSTANT 0u

#define T_UNDEFINED 0x4u /* undefinedp() returns true */

/* utility function for manipulating svalues */

void copy_some_svalues(svalue_t*, svalue_t*, int);
void assign_svalue(svalue_t*, svalue_t*);
void assign_svalue_no_free(svalue_t*, svalue_t*);
/* deferred T_ARRAY/T_CLASS/T_MAPPING/T_PROMISE deallocation (svalue.cc) */
void free_compound(void* ptr, uint32_t type);

#ifdef DEBUG
#define free_svalue(x, y) int_free_svalue(x, y)
#else
#define free_svalue(x, y) int_free_svalue(x)
#endif

// commonly used svalue.
extern svalue_t const0, const1, const0u;

/* These are not used anywhere */

/* Beek - add some sanity to joining strings */
/* add to an svalue */
#define EXTEND_SVALUE_STRING(x, y, z)                                                             \
  SAFE({                                                                                          \
    const auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);                             \
    char* ess_res;                                                                                \
    int ess_len;                                                                                  \
    int ess_r;                                                                                    \
    ess_len = (ess_r = SVALUE_STRLEN(x)) + strlen(y);                                             \
    if (ess_len > max_string_length) error("Maximum string length exceeded in concatenation.\n"); \
    if ((x)->subtype == STRING_MALLOC && MSTR_REF((x)->u.string) == 1) {                          \
      ess_res = (char*)extend_string((x)->u.string, ess_len);                                     \
      if (!ess_res) fatal("Out of memory!\n");                                                    \
      strcpy(ess_res + ess_r, (y));                                                               \
    } else {                                                                                      \
      ess_res = new_string(ess_len, z);                                                           \
      strcpy(ess_res, (x)->u.string);                                                             \
      strcpy(ess_res + ess_r, (y));                                                               \
      free_string_svalue(x);                                                                      \
      (x)->subtype = STRING_MALLOC;                                                               \
    }                                                                                             \
    (x)->u.string = ess_res;                                                                      \
  })

/* <something that needs no free> + string svalue */
#define SVALUE_STRING_ADD_LEFT(y, z)                                                              \
  SAFE({                                                                                          \
    const auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);                             \
    char* pss_res;                                                                                \
    int pss_r;                                                                                    \
    int pss_len;                                                                                  \
    pss_len = SVALUE_STRLEN(sp) + (pss_r = strlen(y));                                            \
    if (pss_len > max_string_length) error("Maximum string length exceeded in concatenation.\n"); \
    pss_res = new_string(pss_len, z);                                                             \
    strcpy(pss_res, y);                                                                           \
    strcpy(pss_res + pss_r, sp->u.string);                                                        \
    free_string_svalue(sp--);                                                                     \
    sp->type = T_STRING;                                                                          \
    sp->u.string = pss_res;                                                                       \
    sp->subtype = STRING_MALLOC;                                                                  \
  })

/* basically, string + string; faster than using extend b/c of SVALUE_STRLEN */
#define SVALUE_STRING_JOIN(x, y, z)                                                               \
  SAFE({                                                                                          \
    const auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);                             \
    char* ssj_res;                                                                                \
    int ssj_r;                                                                                    \
    int ssj_len;                                                                                  \
    ssj_r = SVALUE_STRLEN(x);                                                                     \
    ssj_len = ssj_r + SVALUE_STRLEN(y);                                                           \
    if (ssj_len > max_string_length) error("Maximum string length exceeded in concatenation.\n"); \
    if ((x)->subtype == STRING_MALLOC && MSTR_REF((x)->u.string) == 1) {                          \
      ssj_res = (char*)extend_string((x)->u.string, ssj_len);                                     \
      if (!ssj_res) fatal("Out of memory!\n");                                                    \
      (void)strcpy(ssj_res + ssj_r, (y)->u.string);                                               \
      free_string_svalue(y);                                                                      \
    } else {                                                                                      \
      ssj_res = (char*)new_string(ssj_len, z);                                                    \
      strcpy(ssj_res, (x)->u.string);                                                             \
      strcpy(ssj_res + ssj_r, (y)->u.string);                                                     \
      free_string_svalue(y);                                                                      \
      free_string_svalue(x);                                                                      \
      (x)->subtype = STRING_MALLOC;                                                               \
    }                                                                                             \
    (x)->u.string = ssj_res;                                                                      \
  })

// Translate svalue into json summary, only suitable for
json svalue_to_json_summary(const svalue_t* obj, int depth = 0);

#endif /* LPC_SVALUE_H */
