---
layout: doc
title: floats / to_int
---
# to_int

### NAME

    to_int - convert a float or buffer to an int

### SYNOPSIS

    int to_int( float | string | buffer x);

### DESCRIPTION

    If 'x' is a float, the to_int() call returns the number of type 'int'
    that is equivalent to 'x' (with any decimal stripped off, and floored).

    If 'x' is a buffer, the call returns the integer (in network-byte-order)
    that is embedded in the buffer.

    If 'x' is a string, the call will attempt to convert the string to an
    integer. The conversion will start at the first character and stop before
    the last non-numeric string representation of a number and return.
    If it was unsuccessful, it will return UNDEFINED.

### EXAMPLES

    to_int(3.14) == 3
    to_int(3.89) == 3
    to_int("3.14") == 3
    to_int("3.14abc") == 3
    to_int("abc3.14") == UNDEFINED

### SEE ALSO

    to_float(3), read_buffer(3), sprintf(3), sscanf(3)
