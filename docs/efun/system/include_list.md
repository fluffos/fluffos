---
title: system / include_list
---
# include_list

### NAME

    include_list() - list the files an object #included

### SYNOPSIS

    string *include_list( object obj );

### DESCRIPTION

    Returns an array of filenames of every file the object's program
    actually `#include`d when it was compiled -- nested includes included,
    first-seen order, duplicates dropped. The object's own source file is
    omitted. Each filename carries a leading slash.

    The configured global include file is listed when the compiler opened
    it. An `#include` inside a false `#if` branch is not listed: that file
    was not opened.

    This is the include-side counterpart of `inherit_list()`. Mudlibs that
    rebuild only changed objects can walk `include_list(ob)` to see which
    headers would force `ob` to recompile.

    If no object is supplied, this efun defaults to this_object().

### EXAMPLE

    // Given a file that begins:
    //   #include <globals.h>          // globals.h itself #includes tests.h
    //   #include "/std/room.h"
    include_list(ob);
    // ({ "/include/globals.h", "/include/tests.h", "/std/room.h" })

### SEE ALSO

    inherit_list(3), deep_inherit_list(3), recompile_object(3)
