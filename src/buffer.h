/* buffer.h by John Garnett, 1993/11/07 */

#ifndef _BUFFER_H_
#define _BUFFER_H_

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

INLINE buffer_t *null_buffer PROT((void));
INLINE void free_buffer PROT((buffer_t *));
buffer_t *allocate_buffer PROT((int));
int write_buffer PROT((buffer_t *, int, char *, int));
char *read_buffer PROT((buffer_t *, int, int, int *));

#endif
