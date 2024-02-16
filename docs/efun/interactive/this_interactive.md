---
layout: doc
title: interactive / this_interactive
---
# this_interactive

### NAME

    this_interactive - the current interactive player object

### SYNOPSIS

    object this_interactive( int flag: 1 );
    object this_player( int flag: 0 );
    object this_user( int flag: 0 );

### DESCRIPTION

    Return the object representing the player that caused the calling func‐
    tion to be called.  This returns what this_player() was originally even
    if it changed later due to enable_commands() or command().

    This efun is an alias for `this_player(1)`, refer to the docs for
    this_player(3).

### SEE ALSO

    this_object(3), this_player(3), this_user(3)
