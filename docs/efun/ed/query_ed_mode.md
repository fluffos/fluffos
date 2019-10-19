---
layout: default
title: ed / query_ed_mode
---

### NAME

    query_ed_mode() - find out the status of the current ed session

### SYNOPSIS

    int query_ed_mode()

### DESCRIPTION

    Finds  the  status  of  the  ed  session for the current object, if one
    exists.  It returns:

    0   - the current object is at a normal ed prompt (':')

    -1  - the current object isn't in ed

    -2  - the current object is at the more prompt in the middle of help

    >0  - the object is at a prompt for a line.  The  number  is  the  line
        number.

