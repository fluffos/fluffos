---
layout: default
title: contrib / base_name.pre
---

### NAME

    base_name - return the base name

### SYNOPSIS

    string base_name(string | object ob);

### DESCRIPTION

    returns (file_name(object) | string) up to (but not including) first '#'.
    If no argument is given, <ob> defaults to this_object().

### SEE ALSO

    file_name(3)
