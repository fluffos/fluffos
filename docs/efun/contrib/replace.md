---
title: contrib / replace
---
# replace

### NAME

    replace() - replace occurrences of a substring, or of many substrings

### SYNOPSIS

    string replace( string str, string *|string from, string|void to );

### DESCRIPTION

    Two forms:

    - `replace(str, from, to)` returns `str` with every occurrence of the
      string `from` replaced by `to` (this is the plain string form,
      equivalent to the replace_string() efun).
    - `replace(str, arr)` takes an even-length array `arr` of
      `({ from1, to1, from2, to2, ... })` pairs and applies each
      replacement in turn.

    Errors on an odd-length pair array.

### SEE ALSO

    replace_string(3), replace_dollars(3)
