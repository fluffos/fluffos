---
title: strings / ltrim
---
# ltrim

### NAME

    ltrim() - Remove leading whitespaces (and others)

### SYNOPSIS

    string ltrim( string str );
    string ltrim( string str, string ch);

### DESCRIPTION

    Remove all leading whitespace (or other characters in 'ch') from the
    string 'str' and return a new string.

    The optional second argument is a set of Unicode characters, not a
    set of raw bytes. See trim(3).

### SEE ALSO

    trim(3), rtrim(3)
