#ifndef UTIL_H
#define UTIL_H

#include "hash.h"

void debug_message PROT1V(char *);
FILE *crdir_fopen PROT((char *));

typedef int (*qsort_comparefn_t) PROT((void *, void *));
void quickSort PROT((void *, int, int, qsort_comparefn_t));

typedef struct {
    int real_size;
    char *buffer;
} outbuffer_t;

void outbuf_zero PROT((outbuffer_t *));
void outbuf_add PROT((outbuffer_t *, char *));
void outbuf_addchar PROT((outbuffer_t *, char));
void outbuf_addv PROT2V(outbuffer_t *, char *);
void outbuf_fix PROT((outbuffer_t *));
void outbuf_push PROT((outbuffer_t *));
void outbuf_free PROT((outbuffer_t *));
int outbuf_extend PROT((outbuffer_t *, int));

#endif
