---
layout: doc
title: interactive / this_player
---
# this_player

### NAME

    this_player - the current player object

### SYNOPSIS

    object this_player( int flag: 0 );
    object this_user( int flag: 0 );
    object this_interactive( int flag: 1 );

### DESCRIPTION

    Return the object representing the player that caused the calling func‐
    tion to be called.  Note that  this_player()  may  return  a  different
    value  than this_object() even when called from within a player object.
    If this_player is called as this_player(1) then the returned value will
    be  the  interactive  that  caused  the  calling function to be called.
    this_player(1) may return a different value than this_player() in  cer‐
    tain  cases  (such  as  when  command()  is used by an admin to force a
    player to perform some command).

    `this_user`  and  `this_interactive` are aliases for `this_player` with
    different default flags.

### SEE ALSO

    this_object(3), this_user(3), this_interactive(3), set_this_player(3)
