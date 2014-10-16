#ifndef DEBUGMALLOC_H
#define DEBUGMALLOC_H

struct outbuffer_t;

void *debugmalloc(int, int, const char *);
void *debugrealloc(void *, int, int, const char *);
void *debugcalloc(int, int, int, const char *);
void debugfree(void *);

void debugmalloc_init(void);
void dump_malloc_data(outbuffer_t *);

void set_malloc_mask(int);
char *dump_debugmalloc(const char *, int);

#endif
