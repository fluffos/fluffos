---
layout: default
title: contrib / remove_charmode.pre
---

### NAME

    remove_charmode

### SYNOPSIS

    int remove_charmode(object);

### DESCRIPTION

    switches interactive object into linemode

    returns -1 if argument missing
    -2 in case of error (object destroyed or not interactive)
    current input_to flags with char_mode unset

### SEE ALSO

    query_charmode(3), input_to(3), get_char(3)
