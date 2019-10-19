---
layout: default
title: strings / set_bit
---

### NAME

    set_bit() - set a bit in a bitstring

### SYNOPSIS

    string set_bit( string str, int n );

### DESCRIPTION

    Return  the  new string where bit 'n' is set in string 'str'. Note that
    the old string 'str' is not modified.

    The max value of 'n' is limited. Ask the administrator if you  want  to
    now the maximum value.

    The new string will automatically be extended if needed.

    Bits are packed 6 per byte in printable strings.

### SEE ALSO

    clear_bit(3), test_bit(3)

