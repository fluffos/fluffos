---
title: master / valid_ffi
---
# valid_ffi

### NAME

    valid_ffi - gate every foreign-function-interface operation

### SYNOPSIS

    int valid_ffi( string op, mixed arg, object caller );

### DESCRIPTION

    Every package_ffi operation -- ffi_load(), ffi_symbol(), ffi_prepare(),
    ffi_callback() and ffi_peek() -- is gated by this master apply.  The
    driver calls valid_ffi() before performing the operation and refuses
    to proceed unless it returns a true value.

    Three arguments are passed:

    - `op`     - which operation is being attempted, one of the strings
                 "load", "symbol", "prepare", "callback" or "peek".
    - `arg`    - the operation's principal argument: the library path for
                 "load", the symbol name for "symbol", the function name
                 for "prepare", the address to read for "peek", and 0 for
                 "callback" (which has no such argument).
    - `caller` - the object that invoked the efun (its previous_object()).

    "peek" is gated even though it runs no foreign code: ffi_peek() reads
    arbitrary process memory, a disclosure primitive on its own.  (An
    unmapped address would also crash the driver, so only grant it to
    objects that compute addresses from FFI results, and expect the gate
    to be called once per read -- including from inside C->LPC callback
    comparators.)

    Return a nonzero value to allow the operation; return 0 to deny it, in
    which case the efun raises the runtime error "FFI security violation:
    '<op>' denied by master::valid_ffi."

    The default is secure: if the master object does not define valid_ffi()
    at all, every FFI operation is denied.  A mudlib that wants to use
    package_ffi must implement this apply.

    The "ffi allowed libraries" config option is an independent, coarser
    gate applied to ffi_load() *before* valid_ffi() -- when non-empty it
    restricts ffi_load() to a fixed set of paths.  Leaving it empty imposes
    no path restriction and defers entirely to valid_ffi().

### SEE ALSO

    ffi_load(3), ffi_symbol(3), ffi_prepare(3), ffi_callback(3)
