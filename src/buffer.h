/* buffer.h by John Garnett, 1993/11/07 */

/* It is usually better to include "lpc_incl.h" instead of including this
   directly */

#ifndef _BUFFER_H_
#define _BUFFER_H_

#ifndef NO_BUFFER_TYPE
typedef struct buffer_s {
    /* first two elements of struct must be 'ref' followed by 'size' */
    unsigned short ref;
    unsigned int size;
#ifdef DEBUG
    unsigned short extra_ref;
#endif
    unsigned char item[1];
} buffer_t;

/*
 * buffer.c
 */
extern buffer_t null_buf;

INLINE buffer_t *null_buffer (void);
INLINE void free_buffer (buffer_t *);
buffer_t *allocate_buffer (int);
int write_buffer (buffer_t *, int, const char *, int);
char *read_buffer (buffer_t *, int, int, int *);
#endif

#endif
