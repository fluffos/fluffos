---
layout: default
title: system / find_call_out
---

### NAME

    find_call_out() - find a call out scheduled to be called next

### SYNOPSIS

    int find_call_out( string func ):

### DESCRIPTION

    Find  the  first  call  out due to be executed for function 'func', and
    return the time left. If it is not found, then return -1.

### SEE ALSO

    call_out(3), remove_call_out(3), set_heart_beat(3)

