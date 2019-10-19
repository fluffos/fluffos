---
layout: default
title: internals / dump_prog
---

### NAME

    dump_prog() - dump/disassemble an LPC object

### SYNOPSIS

    void dump_prog( object ob, int flags default: 0, string file );

### DESCRIPTION

    dump_prog()  dumps  information  about  the program of 'obj' to a file,
    'file', or "/PROG_DUMP" if 'file' is not given.  If the current  object
    does not have write access to the file, it fails.

    Flags  can be a combination of the following values: 1 - include a dis‐
    assembly of the i-code 2 - include line number information

### SEE ALSO

    debug_info(3), dumpallobj(3)

