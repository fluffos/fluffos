---
layout: default
title: stdlib / all_environment
---

### NAME

    all_environment() - return all environments of an object

### SYNOPSIS

    object* all_environment( void|object ob );

### DESCRIPTION

    This function will return a list of objects, from bottom to top, that
    comprise the environments for object `ob`.

    If no object is passed, then `previous_object()` will be used.

### EXAMPLE

    If a player has a coin in a bag, and youi ask for all_environment() on
    the the coin, it will return:

    ({ bag, player, room })

### SEE ALSO

    deep_inventory
