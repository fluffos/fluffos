---
layout: default
title: strings / sprintf
---

### NAME

    printf, sprintf - formatted output conversion

### SYNOPSIS

    void printf( string format, ... );
    string sprintf( string format, ... );

### DESCRIPTION

    An  implementation  of (s)printf() for LPC, with quite a few extensions
    Implemented by Lynscar (Sean A Reith).

    This version supports the following as modifiers:

    " "     pad positive integers with a space.

    "+"     pad positive integers with a plus sign.

    "-"     left adjusted within field size.
            NB: std (s)printf() defaults to right justification,  which  is
            unnatural  in the context of a mainly string based language but
            has been retained for "compatability".

    "|"     centered within field size.

    "="     column mode if strings are greater than  field  size.  This  is
            only meaningful with strings, all other types ignore this. Col‐
            umns are auto-magically word wrapped.

    "#"     table mode, print a list of '\n' separated 'words' in  a  table
            within the field size.  only meaningful with strings.

    n       specifies  the  field  size,  a '*' specifies to use the corre‐
            sponding arg as the field size.  If n is prepended with a zero,
            then  is padded zeros, else it is padded with spaces (or speci‐
            fied pad string).

    "."n    precision of n, simple strings truncate after this  (if  preci‐
            sion  is greater than field size, then field size = precision),
            tables use precision to specify the number of columns (if  pre‐
            cision  not  specified  then  tables calculate a best fit), all
            other types ignore this.

    ":"n    n specifies the fs _and_ the precision, if n is prepended by  a
            zero then it is padded with zeros instead of spaces.

    "@"     the argument is an array.  the corresponding format_info (minus
            the "@") is applyed to each element of the array.

    "'X'"   The char(s) between the single-quotes are used to pad to  field
            size  (defaults  to  space)  (if both a zero (in front of field
            size) and a pad string are specified, the one specified  second
            overrules).   NOTE:  to include "'" in the pad string, you must
            use "\'" (as the backslash has to be escaped  past  the  inter‐
            preter), similarly, to include "\" requires "\\".

    The following are the possible type specifiers.

    %       in  which  case  no  arguments  are  interpreted,  and a "%" is
            inserted, and all modifiers are ignored.

    O       the argument is an LPC datatype.

    s       the argument is a string.

    d, i    the integer arg is printed in decimal.

    c       the integer arg is to be printed as a character.

    o       the integer arg is printed in octal.

    x       the integer arg is printed in hex.

    X       the integer arg is printed in hex (with A-F in capitals).

    f       floating point number

### RETURN VALUES

    sprintf() returns the formatted string.

### AUTHOR

    Sean A. Reith (Lynscar)

### SEE ALSO

    sscanf(3)

