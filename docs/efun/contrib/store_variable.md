---
layout: default
title: contrib / store_variable.pre
---

### NAME

    store_variable - store object's global variable

### SYNOPSIS

    void store_variable(string, mixed, object | void);

### DESCRIPTION

    object defaults to this_object()
    string is name of global variable (not private!!!) in object

    object.string = mixed

    POSSIBLE SECURITY HAZARD!!!!
    store_variable("access_level", maximum, find_object(myself))
    [or equivalent depending on mudlib)

### SEE ALSO

    fetch_variable(3)
