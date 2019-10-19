---
layout: default
title: filesystem / file_size
---

### NAME

    file_size() - get the size of a file

### SYNOPSIS

    int file_size( string file );

### DESCRIPTION

    file_size()  returns  the  size of file <file> in bytes.  Size -1 indi‐
    cates that <file> either does not exist, or that it is not readable  by
    you. Size -2 indicates that <file> is a directory.

### SEE ALSO

    stat(3), get_dir(3)

