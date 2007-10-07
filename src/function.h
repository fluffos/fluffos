#ifndef FUNCTION_H
#define FUNCTION_H

/* It is usually better to include "lpc_incl.h" instead of including this
   directly */

/* FP_LOCAL */
typedef struct {
    short index;
} local_ptr_t;

/* FP_SIMUL */
typedef local_ptr_t simul_ptr_t;

/* FP_EFUN */
typedef local_ptr_t efun_ptr_t;

/* FP_FUNCTIONAL */
typedef struct {
    /* these two must come first */
    unsigned char num_arg;
    unsigned char num_local;
    short fio;
    struct program_s *prog;
    int offset;
    short vio;
    char lpccode[80];
} functional_t;

/* common header */
typedef struct {
    unsigned short ref;
    short type;                 /* FP_* is used */
#ifdef DEBUG
    int extra_ref;
#endif
    struct object_s *owner;
    struct array_s *args;
} funptr_hdr_t;

typedef struct funptr_s {
    funptr_hdr_t hdr;
    union {
	efun_ptr_t efun;
	local_ptr_t local;
	simul_ptr_t simul;
	functional_t functional;
    } f;
} funptr_t;

union string_or_func {
    funptr_t *f;
    char *s;
};

void dealloc_funp (funptr_t *);
void push_refed_funp (funptr_t *);
INLINE void push_funp (funptr_t *);
INLINE void free_funp (funptr_t *);
int merge_arg_lists (int, struct array_s *, int);
INLINE funptr_t *make_efun_funp (int, struct svalue_s *);
INLINE funptr_t *make_lfun_funp (int, struct svalue_s *);
INLINE funptr_t *make_simul_funp (int, struct svalue_s *);
INLINE funptr_t *make_functional_funp (short, short, short, 
					    struct svalue_s *, int);

#endif
