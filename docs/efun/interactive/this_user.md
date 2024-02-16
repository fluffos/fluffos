---
layout: doc
title: interactive / this_user
---
# this_user

### NAME

    this_user - the current interactive player object

### SYNOPSIS

    object this_user( int flag: 0 );
    object this_player( int flag: 0 );
    object this_interactive( int flag: 1 );

### DESCRIPTION

    Return the object representing the player that caused the calling func‐
    tion to be called.

    This efun is an alias for `this_player(0)`, refer to the docs for
    this_player(3).

### SEE ALSO

    this_object(3), this_player(3), this_interactive(3), set_this_player(3)