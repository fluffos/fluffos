---
title: strings / trim
---
# trim

### NAME

    trim() - Remove leading and trailing whitespaces

### SYNOPSIS

    string trim( string str );
    string trim( string str, string ch);

### DESCRIPTION

    Remove all leading and trailing whitespace (or other characters in
    'ch') from the string 'str' and return a new string.

    The optional second argument is a set of Unicode characters, not a
    set of raw bytes. `trim("《三字经》", "　")` leaves the title intact
    -- U+3000 and U+300A share a UTF-8 prefix, but they are different
    characters.

### SEE ALSO

    ltrim(3), rtrim(3)
