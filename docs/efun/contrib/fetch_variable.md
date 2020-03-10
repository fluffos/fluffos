---
layout: default
title: contrib / fetch_variable.pre
---

### NAME

    fetch_variable

### SYNOPSIS

    mixed fetch_variable(string, object | void);

### DESCRIPTION

    object defaults to this_object()
    string is name of global variable (not private!!!) in object

    returns object.string

    POSSIBLE SECURITY HAZARD!!!!

### SEE ALSO

    store_variable(3)
