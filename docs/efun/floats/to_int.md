---
layout: default
title: floats / to_int
---

### NAME

    to_int - convert a float or buffer to an int

### SYNOPSIS

    int to_int( float | buffer x );

### DESCRIPTION

    If  'x'  is a float, the to_int() call returns the number of type 'int'
    that is equivalent to 'x' (with any decimal stripped off).  If 'x' is a
    buffer,  the  call  returns the integer (in network-byte-order) that is
    embedded in the buffer.

### SEE ALSO

    to_float(3), read_buffer(3), sprintf(3), sscanf(3)

