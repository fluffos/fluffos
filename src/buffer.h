/* buffer.h by John Garnett, 1993/11/07 */

#ifndef _BUFFER_H_
#define _BUFFER_H_

struct buffer {
    /* first two elements of struct must be 'ref' followed by 'size' */
    unsigned short ref;
    unsigned int size;
    unsigned char item[1];
};

/*
 * buffer.c
 */
extern struct buffer null_buf;

struct buffer *null_buffer PROT((void));
void free_buffer PROT((struct buffer *));
struct buffer *allocate_buffer PROT((int));
int write_buffer PROT((struct buffer *, int, char *, int));
char *read_buffer PROT((struct buffer *, int, int, int *));

#endif
