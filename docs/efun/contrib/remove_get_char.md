---
layout: default
title: contrib / remove_get_char.pre
---

### NAME

    remove_get_char

### SYNOPSIS

    int remove_get_char(object);

### DESCRIPTION

    cancels active input_to
    set's interactive->carryover = NULL
    set's interactive->num_carry = 0

    returns -3 if argument missing
    -2 if object destroyed or not interactive
    -1 if no input_to active
    1 otherwise

#### QUESTION

    - what's the meankng of
    interactive->carryover?
    interactive->num_carry?
