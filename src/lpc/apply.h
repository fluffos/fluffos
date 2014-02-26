#ifndef LPC_APPLY_H_
#define LPC_APPLY_H_

typedef struct svalue_s svalue_t;
typedef struct object_s object_t;

// Global static result for apply() ,
// used by routines that want to return a pointer to an svalue
extern svalue_t apply_ret_value;

// Call a specific function on a specific object
// Result are stored in a global value, no need to free.
svalue_t *safe_apply(const char *, object_t *, int, int);

// Unsafe version, should only be used in efuns.
svalue_t *apply(const char *, object_t *, int, int);

// TODO: Some place still use this function.
// because apply() would reset the value on next call
int apply_low(const char *, object_t *, int);

#endif /* LPC_APPLY_H_ */
