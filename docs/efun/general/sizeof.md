---
title: general / sizeof
---
# sizeof

### NAME

    sizeof()  -  return the number of elements in an array, mapping, buffer
    or string

### SYNOPSIS

    int sizeof( mixed var );

### DESCRIPTION

    Return the number of elements in an array, mapping,  string  or  buffer
    'var'.  If 'var' is not an array, mapping, string, or buffer, then zero
    (0) is returned.

    For a string, the element count is the number of UTF-8 characters
    (extended grapheme clusters), exactly like strlen(3) — not the number
    of bytes. For a buffer it is the number of bytes.

### SEE ALSO

    allocate(3), allocate_mapping(3), strlen(3)

