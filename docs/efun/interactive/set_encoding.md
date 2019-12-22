---
layout: default
title: interactive / set_encoding
---

### SYNOPSIS

    string set_encoding( string encoding | void );

### DESCRIPTION

    set output/input encoding for current player.

    If given encoding name is not avaiable, a error will be thrown. The avaiable encoding name
    depends on your ICU version, Generally, use GBK for chinese.

    If no argument present, reset the player to no encoding, which means UTF-8.

    Returns the canonical encoding name from ICU, and it will be the same as query_encoding() returns.

### SEE ALSO

    query_encoding(3)

