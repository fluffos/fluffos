---
layout: default
title: parsing / parse_init
---

### NAME

    parse_init()

### SYNOPSIS

    void parse_init();

### DESCRIPTION

    The efun parse_init() is used to tell MudOS that this object is one
    that may use or be used by the parsing package. If your object does
    not call this then trying to use other parsing efuns will generate a
    runtime error and the parser will ignore the object when searching
    for matches. Suggest call parse_init() from create() in the standard
    object.

### SEE ALSO

    parse_refresh(3)

