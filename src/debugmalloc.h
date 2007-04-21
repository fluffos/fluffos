#ifndef DEBUGMALLOC_H
#  define DEBUGMALLOC_H
#  ifdef DEBUGMALLOC
void *debugmalloc (int, int, char *);
void *debugrealloc (void *, int, int, char *);
void *debugcalloc (int, int, int, char *);
void debugfree (void *);

void debugmalloc_init (void);
void dump_malloc_data (outbuffer_t *);

#    ifdef DEBUGMALLOC_EXTENSIONS
void set_malloc_mask (int);
char *dump_debugmalloc (char *, int);
#    endif
#  else
   /* not DEBUGMALLOC */
#    undef DEBUGMALLOC_EXTENSIONS
#    undef CHECK_MEMORY
#  endif
#endif
