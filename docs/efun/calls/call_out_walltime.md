---
layout: default
title: calls / call_out_walltime
---

### NAME

    call_out_walltime - delayed function call in same object

### SYNOPSIS

    int call_out_walltime( string | function fun, int | float delay, mixed arg ... );

### DESCRIPTION

    This efun is identical to call_out except it does not schedule the
    call on the game loop. Rather, in real seconds. The delay can be in
    seconds or fraction of a second.

### SEE ALSO

    remove_call_out, call_out_info, call_out

