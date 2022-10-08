---
layout: default
title: contrib / remove_shadow.pre
---

### NAME

    remove_shadow() - remove all shadows from the specified object, or remove
    a specified shadow from another object. does not destruct the shadow.

### SYNOPSIS

    int remove_shadow(object ob);

### DESCRIPTION

    ob defaults to this_object()
    returns 0 if object is either destroyed or is no shadow and isn't shadowed
    returns 1 otherwise

    ob <- shadow1 <- shadow2

    remove(shadow2): ob <- shadow1 shadow2
    remove(shadow1): ob <- shadow2 shadow1
    remove(ob): shadow1 <- shadow2 ob

    #ifndef NO_SHADOWS
