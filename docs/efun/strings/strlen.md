---
layout: default
title: efuns / strings / strlen
---

### NAME

    strlen()

### SYNOPSIS

    int strlen( string str );

### DESCRIPTION

    strlen() returns the number of characters in the string 'str'.

    LPC strings are UTF-8 encoded. Driver calcuate numbers of characters according to the Unicode concept
    "Extended Grapheme Cluster", as defined in "https://www.unicode.org/reports/tr29/".

    strlen("abc") == 3
    strlen("你好") == 2

### SEE ALSO

    sizeof(3)
