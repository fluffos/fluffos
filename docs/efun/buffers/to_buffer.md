---
title: buffers / to_buffer
---
# to_buffer

### NAME

    to_buffer() - convert a string or an array of ints to a buffer

### SYNOPSIS

    buffer to_buffer( string | buffer | mixed *value );

### DESCRIPTION

    Returns a buffer holding the bytes of <value>:

    - A string contributes its raw UTF-8 bytes (a multi-byte character
      becomes several bytes).
    - An array must contain only ints in the range 0..255; each element
      becomes one byte. Any other item raises an error.
    - A buffer is returned unchanged.

    This efun is also the implicit promotion used by the compiler, so a
    string or an array of ints can be used directly wherever a buffer is
    expected in an assignment context:

    - `buffer b = "abc";` and `b = ({ 1, 2, 255 });`
    - `b += "abc";` and `b + ({ 0 })` (appends the converted bytes)
    - `b[i..j] = "abc";` (buffer range assignment)

### EXAMPLES

    ```c
    buffer b;

    b = to_buffer("é");        // ({ 0xC3, 0xA9 }) -- 2 UTF-8 bytes
    b = to_buffer(({ 7, 255 }));
    b = to_buffer(({ 300 }));  // error: item out of 0..255
    ```

### SEE ALSO

    allocate_buffer(3), bufferp(3), read_buffer(3), write_buffer(3),
    to_int(3), to_float(3)
