---
layout: default
title: pcre / pcre_replace_callback
---

### NAME

    pcre_replace_callback() - string replace uses a callback to get the replace string

### SYNOPSIS

    string pcre_replace_callback(string, string, string | function, ...);

### DESCRIPTION

    returns a string where all captured groups have been replaced by the
    return value of function pointe fun or function fun in object ob.
    (called with the matched string and match number, starting with 0)

### SEE ALSO

    pcre_assoc(3), pcre_cache(3), pcre_extract(3), pcre_replace(3)
