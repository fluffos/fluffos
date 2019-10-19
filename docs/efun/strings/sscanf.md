---
layout: default
title: strings / sscanf
---

### NAME

    sscanf() - match substrings in a string.

### SYNOPSIS

    int sscanf( string str, string fmt,
                mixed var1, mixed var2 ... );

### DESCRIPTION

    Parse  a string 'str' using the format 'fmt'. 'fmt' can contain strings
    separated by "%d" and "%s". Every "%d" and "%s" corresponds to  one  of
    'var1',  'var2'...   "%d"  will  give  a  number,  and "%s" will give a
    string.  The * may be used in a format specifier (e.g. %*d and %*s)  to
    allow  integers  or  strings  (respectively)  to be skipped over in the
    input string (without being assigned to a variable).  The LPC  sscanf()
    is similar to its C counterpart however it does behave somewhat differ‐
    ently.  It is not necessary (or possible) to pass the address of  vari‐
    ables into sscanf (simply pass the name of the variable).  Another dif‐
    ference is that in the LPC sscanf(), sscanf(str, "%s %s",  str1,  str2)
    will  parse  the  first  word in str into str1 and the remainder of str
    into str2.

    The number of matched "%d" and "%s" is returned.

### SEE ALSO

    explode(3), replace_string(3), strsrch(3)

