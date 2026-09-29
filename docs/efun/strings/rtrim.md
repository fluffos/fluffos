---
title: strings / rtrim
---
# rtrim

### NAME

    rtrim() - Remove trailing whitespaces (and others)

### SYNOPSIS

    string rtrim( string str );
    string rtrim( string str, string ch);

### DESCRIPTION

    Remove all trailing whitespace (or other characters in 'ch') from the
    string 'str' and return a new string.

    The optional second argument is a set of Unicode characters, not a
    set of raw bytes. See trim(3).

### SEE ALSO

    trim(3), ltrim(3)
