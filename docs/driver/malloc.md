---
layout: doc
title: general / malloc
---
# malloc

Note: some machines don't allow the system calls to be replaced with
a function of the same name. In particular, the compile will fail during
the link phase with a 'duplicate symbol' error. Most of the machines
I've seen that exhibit this behavior use shared libraries (NeXT, RS/6000).
Also note that smalloc.c uses sbrk() and brk() and these routines cannot
be used in a program that uses malloc() (read the sbrk() man page for more
information). Even if the program doesn't explicitly call malloc, it will
fail if it calls system calls that internally use malloc. Thus the only
truly safe way to use sbrk and brk is to replace the definition of system
malloc. And some machines do not allow system malloc to be replaced.

Note: if you plan to hack on the driver, you should use MALLOC(x, ...),
FREE(x, ...), REALLOC(x, ...), and CALLOC(x, ...) instead of the lowercase
counterparts. Be sure to look at how these macros are used in the driver
source itself in order to figure out the right syntax. The uppercase
versions are macros that get replaced with the proper function names
depending upon which malloc package is chosen. This change was made in
order to provide for those machines that complain when functions are added
with the same names as system calls. The change also allows some stats to
be collected even when using system malloc. Various debug version of these
macros also exist. Look at the macros defined near the end of the file
config.h in the src directory.
