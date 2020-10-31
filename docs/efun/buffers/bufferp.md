---
layout: default
title: buffers / bufferp
---

### NAME

    bufferp() - identifies whether a given variable is a buffer

### SYNOPSIS

    int bufferp( mixed arg );

### DESCRIPTION

    Return 1 if 'arg' is a buffer value and zero (0) otherwise.

### EXAMPLE

    int is_buffer = bufferp( allocate_buffer(10) ); // 1
    int is_buffer = bufferp( "Foo" ); // 0    

### SEE ALSO

    mapp(3),  stringp(3),  pointerp(3),  objectp(3),  intp(3),  bufferp(3),
    floatp(3), functionp(3), nullp(3), undefinedp(3), errorp(3)

