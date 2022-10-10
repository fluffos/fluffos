---
layout: default
title: strings / sscanf
---

### NAME

    sscanf() - match substrings in a string.

### SYNOPSIS

    int sscanf( string input, string fmt, ... );

### DESCRIPTION

    Parse a string 'input' using the string format 'fmt', which can contain
    words separated by specifiers.  Every specifier corresponds to a match-
    ing function argument passed into  sscanf()  after the string 'fmt', in
    order of appearance and assigning the matched values By Reference.  The
    number of specifiers included in 'fmt' should match the number of func-
    tion arguments included after string 'fmt'.

    Specifiers:
        %s          -   match a string
        %d          -   match an integer
        %f          -   match a float
        %x          -   match a base 16 number and convert to base 10
        %(regexp)   -   match a regexp pattern

    The "*" may be used in a specifier (e.g. "%*s") to skip over a match in
    the input string and not assign it to a function argument.   A "%%" may
    be used to match the "%" character,  which will also be skipped over in
    the function arguments.

    The LPC sscanf() is similar to its C counterpart however it does behave
    somewhat differently.  It is not necessary or possible to pass the add-
    ress of  variables into sscanf  (simply pass the name of the variable).

    Another difference is that  in the LPC  sscanf(),  sscanf(str, "%s %s",
    str1, str2)  will parse the  first word in  str into str1  and the rem-
    ainder of str into str2.
    
    The "%s" specifier can match an empty string, which can be an undesired
    result.  sscanf(" ", "%s %s", str1, str2)  will return 2, with str1 and
    str2 being assigned an "" empty string.

### RETURN VALUES

    The number of matched specifiers is returned.

    All matched specifiers are assigned  By Reference to function arguments
    included after string 'fmt', in order of appearance.

### EXAMPLE

    Basic Usage:
        string what, who;
        
        if (sscanf(input, "give %s to %s", what, who) == 2)
            write("You give " + what + " to " + who + ".");
        else
            write("Give what to who?");

        sscanf("give", "give %s to %s", what, who) == 0
        what == UNDEFINED && who == UNDEFINED

        sscanf("give item", "give %s to %s", what, who) == 1
        what == "item" && who == UNDEFINED

        sscanf("give item to name", "give %s to %s", what, who) == 2
        what == "item" && who == "name"

    Numeric:
        int i;
        sscanf("123", "%d", i) == 1
        i == 123

        float f;
        sscanf("1.23", "%f", f) == 1
        f == 1.230000

        int b10;
        sscanf("ABC", "%x", b10) == 1
        b10 == 2748

    Regexp:
        string str1, str2;
        sscanf("one two", "%([a-z]+) %([a-z]+)", str1, str2) == 2
        str1 == "one" && str2 == "two"

### SEE ALSO

    explode(3), pcre_extract(3), regexp(3), replace_string(3), strsrch(3)

