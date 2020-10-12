---
layout: default
title: arrays / pointerp
---

### NAME

    pointerp() - identifies whether a given variable is an array

### SYNOPSIS

    int pointerp( mixed arg );

### DESCRIPTION

    Returns 1 if 'arg' is an array, otherwise returns 0.
    
### EXAMPLE

    int is_array = pointerp( ({ 1, 2, 3, 4 }) ); // 1
    int is_array = pointerp( "Foo" ); // 0

### SEE ALSO

    mapp(3),  stringp(3), objectp(3), intp(3), bufferp(3), floatp(3), func‐
    tionp(3), nullp(3), undefinedp(3), errorp(3), arrayp(3)

