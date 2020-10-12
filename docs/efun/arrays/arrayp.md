---
layout: default
title: arrays / arrayp
---

### NAME

    arrayp() - identifies whether a given variable is an array

### SYNOPSIS

    int arrayp( mixed arg );

### DESCRIPTION

    Returns 1 if 'arg' is an array, otherwise returns 0.

### EXAMPLE

    int is_array = arrayp( ({ 1, 2, 3, 4 }) ); // 1
    int is_array = arrayp( "Foo" ); // 0

### SEE ALSO

    mapp(3),  stringp(3), objectp(3), intp(3), bufferp(3), floatp(3), func‐
    tionp(3), nullp(3), undefinedp(3), errorp(3), pointerp(3)

