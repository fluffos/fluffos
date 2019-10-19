---
layout: default
title: interactive / remove_action
---

### NAME

    remove_action - unbind a command verb from a local function

### SYNOPSIS

    int remove_action( string fun, string cmd );

### DESCRIPTION

    remove_action(3)  unbinds a verb cmd from an object function fun. Basi‐
    cally,  remove_action()  is  the  complement   to   add_action(3)   and
    add_verb(3).  When a verb is no longer required, it can be unbound with
    remove_action().

### RETURN VALUES

    remove_action() returns:

        1 on success.

        0 on failure.

### SEE ALSO

    add_action(3), query_verb(3), init(4)

