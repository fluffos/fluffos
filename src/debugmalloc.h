#ifndef DEBUGMALLOC_H
#  define DEBUGMALLOC_H
#  ifdef DEBUGMALLOC
void *debugmalloc PROT((int, int, char *));
void *debugrealloc PROT((void *, int, int, char *));
void *debugcalloc PROT((int, int, int, char *));
void debugfree PROT((void *));

void debugmalloc_init PROT((void));
void dump_malloc_data PROT((outbuffer_t *));

#    ifdef DEBUGMALLOC_EXTENSIONS
void set_malloc_mask PROT((int));
char *dump_debugmalloc PROT((char *, int));
#    endif
#  else
   /* not DEBUGMALLOC */
#    undef DEBUGMALLOC_EXTENSIONS
#    undef CHECK_MEMORY
#  endif
#endif
