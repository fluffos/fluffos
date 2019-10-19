---
layout: default
title: mappings / match_path
---

### NAME

    match_path() - search a mapping for a path

### SYNOPSIS

    mixed match_path( mapping m, string str );

### DESCRIPTION

    match_path()  searches a mapping for a path.  Each key is assumed to be
    a string.  The value is  completely  arbitrary.   The  efun  finds  the
    largest matching path in the mapping.  Keys ended in '/' are assumed to
    match paths with character that follow the '/', i.e. /  is  a  wildcard
    for anything below this directory.

### AUTHOR

    Cygnus

