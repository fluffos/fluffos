package develop;

    mixed debug_info(int, object);
    int refs(mixed);

/* dump_prog: disassembler... comment out this line if you don't want the
   disassembler compiled in.
*/
    void dump_prog(object,...);

#if defined(PROFILING) && defined(HAS_MONCONTROL)
    void moncontrol(int);
#endif

#if (defined(DEBUGMALLOC) && defined(DEBUGMALLOC_EXTENSIONS))
    void debugmalloc(string, int);
    void set_malloc_mask(int);
    void check_memory(int default: F_CONST0);
#endif

#ifdef TRACE
    string traceprefix(string | int);
    int trace(int);
#endif

#if defined(NeXT) && defined(NEXT_MALLOC_DEBUG)
    int malloc_check();
    int malloc_debug(int);
#endif
