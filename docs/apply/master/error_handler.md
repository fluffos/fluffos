---
layout: default
title: master / error_handler
---

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
    The 'caught' flag is 1 if the error was trapped by catch().

### SEE ALSO

    catch(3), error(3), throw(3), log_error(4)

### AUTHOR

    Beek

