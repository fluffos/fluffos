
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
    string debugmalloc(string, int default: 0);
    void set_malloc_mask(int);
    string check_memory(int default: 0);
#endif
    string dump_stralloc(string);
#ifdef DEBUG
    string traceprefix(string | int);
    int trace(int);
#endif
