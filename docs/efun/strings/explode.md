---
layout: default
title: strings / explode
---

### NAME

    explode() - break up a string

### SYNOPSIS

    string *explode( string str, string del );

### DESCRIPTION

    explode() returns an array of strings, created when the string <str> is
    split into pieces as divided by the delimiter <del>.

### EXAMPLE

    explode(str," ") will return as an array all of the words (separated by
    spaces) in the string <str>.

### SEE ALSO

    sscanf(3), extract(3), replace_string(3), strsrch(3)

