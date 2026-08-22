---
title: calls / call_out_walltime
---
# call_out_walltime

### NAME

    call_out_walltime - delayed function call in same object

### SYNOPSIS

    int call_out_walltime( string | function fun, int | float delay, mixed arg ... );
    promise call_out_walltime( int | float delay );

### DESCRIPTION

    This efun is identical to call_out except it does not schedule the
    call on the game loop. Rather, in real seconds. The delay can be in
    seconds or fraction of a second.

    The delay-only form returns a promise exactly as call_out(3)'s does,
    measured in wall time.

### SEE ALSO

    remove_call_out, call_out_info, call_out

