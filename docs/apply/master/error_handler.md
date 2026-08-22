---
title: master / error_handler
---
# error_handler

### NAME

    error_handler - function in master object to handle errors

### SYNOPSIS

    void error_handler( mapping error, int caught );

### DESCRIPTION

    This function allows the mudlib to handle errors instead of the driver.
    The contents of the 'error' mapping are:

        ([
            "error"   : string,     // the error
            "program" : string,     // the program
            "object"  : object,     // the current object
            "line"    : int,        // the line number
            "trace"   : mapping*    // a trace back
        ])

    Each line of traceback is a mapping containing the following:

        ([
            "function"  : string,   // the function name
            "program"   : string,   // the program
            "object"    : object,   // the object
         "file"      : string,   // the file to which the line number refers
            "line"      : int       // the line number
        ])

    Not every entry has all of those. A frame entered from DRIVER CONTEXT
    -- a promise reaction handler, the defer() teardown of an abandoned
    coroutine -- is running no program in no object, and its entry contains
    ONLY "function" (the string "CATCH" for a catch marker, "<driver>"
    otherwise): no "program", "object", "file" or "line". Index those keys
    defensively rather than assuming they are present.

    The 'caught' flag is 1 if the error was trapped by catch().

### SEE ALSO

    catch(3), error(3), throw(3), log_error(4)

### AUTHOR

    Beek

