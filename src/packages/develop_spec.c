#include "spec.h"

    mixed debug_info(int, object);
    int refs(mixed);
#ifdef DEBUG
    mixed *destructed_objects();
#endif

/* dump_prog: disassembler... comment out this line if you don't want the
   disassembler compiled in.
*/
    void dump_prog(object,...);

#if defined(PROFILING) && defined(HAS_MONCONTROL)
    void moncontrol(int);
#endif

#if (defined(DEBUGMALLOC) && defined(DEBUGMALLOC_EXTENSIONS))
    string debugmalloc(string, int);
    void set_malloc_mask(int);
    string check_memory(int default: 0);
#endif

#ifdef TRACE
    string traceprefix(string | int);
    int trace(int);
#endif

#if defined(NeXT) && defined(NEXT_MALLOC_DEBUG)
    int malloc_check();
    int malloc_debug(int);
#endif
