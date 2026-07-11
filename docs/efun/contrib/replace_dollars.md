---
title: contrib / replace_dollars
---
# replace_dollars

### NAME

    replace_dollars() - expand $-markers in a string

### SYNOPSIS

    string replace_dollars( string str, string *pairs );

### DESCRIPTION

    Scans `str` for `$`-prefixed markers and substitutes them using
    `pairs`, an even-length array alternating a marker and its
    replacement text, e.g.
    `({ "$name", "Bob", "$place", "the docks" })`. Text that matches no
    marker is copied through unchanged; if nothing matches, the original
    string is returned.

### SEE ALSO

    replace(3), replace_string(3)
