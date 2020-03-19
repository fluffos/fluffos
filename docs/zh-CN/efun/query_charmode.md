---
layout: default
title: contrib / query_charmode.pre
---

### NAME

    query_charmode

### SYNOPSIS

    int query_charmode(object);

### DESCRIPTION

    returns -1 if argument missing
    -2 in case of error (object destroyed or not interactive)
    0 if interactive object is in line mode
    !0 otherwise (actual value depends on compile time define)

### SEE ALSO

    remove_charmode(3), input_to(3), get_char(3)
