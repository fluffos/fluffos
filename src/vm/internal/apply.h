#ifndef LPC_APPLY_H_
#define LPC_APPLY_H_

enum origin {
  ORIGIN_DRIVER = 0x01,
  ORIGIN_LOCAL = 0x02,
  ORIGIN_CALL_OTHER = 0x04,
  ORIGIN_SIMUL_EFUN = 0x08,
  ORIGIN_INTERNAL = 0x10,
  ORIGIN_EFUN = 0x20,
  /* pseudo frames for call_other function pointers and efun pointer */
  ORIGIN_FUNCTION_POINTER = 0x40,
  /* anonymous functions */
  ORIGIN_FUNCTIONAL = 0x80
};

inline const char *origin_to_name(const int origin) {
  switch (origin) {
    case ORIGIN_DRIVER:
      return "driver";
    case ORIGIN_LOCAL:
      return "local";
    case ORIGIN_CALL_OTHER:
      return "call_other";
    case ORIGIN_SIMUL_EFUN:
      return "simul_efun";
    case ORIGIN_INTERNAL:
      return "internal";
    case ORIGIN_EFUN:
      return "efun";
    case ORIGIN_FUNCTION_POINTER:
      return "function_pointer";
    case ORIGIN_FUNCTIONAL:
      return "functional";
    default:
      return "unknown";
  }
}

// Global static result for apply() ,
// used by routines that want to return a pointer to an svalue
extern struct svalue_t apply_ret_value;

// Call a specific function on a specific object
// Result are stored in a global value, no need to free.
svalue_t *safe_apply(const char *, struct object_t *, int, int);

// Unsafe version, should only be used in efuns.
svalue_t *apply(const char *, struct object_t *, int, int);

// TODO: Some place still use this function.
// because apply() would reset the value on next call
int apply_low(const char *, struct object_t *, int);

#endif /* LPC_APPLY_H_ */
