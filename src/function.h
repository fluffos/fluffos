#ifndef FUNCTION_H
#define FUNCTION_H

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
#ifndef LPC_TO_C
    short offset;
#else
    POINTER_INT offset;
#endif
    program_t *prog;
    short fio, vio;
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

void dealloc_funp PROT((funptr_t *));
void push_refed_funp PROT((funptr_t *));

#endif
