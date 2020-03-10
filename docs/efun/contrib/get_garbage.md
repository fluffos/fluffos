---
layout: default
title: contrib / get_garbage.pre
---

### NAME

    get_garbage

### SYNOPSIS

    object *get_garbage();

### DESCRIPTION

    returns array of all (up to **MAX_ARRAY_SIZE**) those cloned(!) objects which
    have neither environment nor inventory and aren't shadowing another object
    and haven't object->super set
