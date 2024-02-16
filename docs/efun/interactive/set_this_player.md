---
layout: doc
title: interactive / set_this_player
---
# set_this_player

### NAME

    set_this_player - change the current player object

### SYNOPSIS

    void set_this_player( object who );
    void set_this_user( object who );

### DESCRIPTION

    set_this_player()   causes   'who'   to   become   the   new  value  of
    this_player().  This efun  is  only  available  with  __NO_ADD_ACTION__
    defined,  and is included since with __NO_ADD_ACTION__ parsing and dis‐
    patching of commands is the mudlib's responsibility.

    'who' can also be zero, in which this_player() is set to zero.

### SEE ALSO

    this_object(3), this_player(3), this_interactive(3)
