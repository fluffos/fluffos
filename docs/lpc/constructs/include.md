---
layout: doc
title: constructs / include
---
# include

### The LPC #include directive:

Syntax:

    #include <file.h>

Alternate:

    #include "file.h"

Note: the `#include "file.h"` form looks for file.h in the current directory.
The `#include <file.h>` form looks for file.h in one of the standard system
include directories (on TMI these directories are /include and /local/include).

For those that know C, the LPC #include statement is identical to C's #include
statement.

For those that don't know C, the #include statement is a way to textually
include one file into another. Putting a statement `#include "file.h"` in a
file gives the same effect as if you had simply typed the contents of file.h
directly into the file at the point where you had the #include statement.

Included files are recompiled each time the object that include's them is
recompiled. If the included file contains variables or functions of the same
name as variables in the file doing the including, then a duplicate-name
error will occur at compile time(in the same way that the error would occur
if you simply typed in file.h rather than using #include).
