---
layout: default
title: object / init
---

### NAME

    init  -  function  in  an  object called by move_object() to initialize
    verb/actions

### SYNOPSIS

    void init( void );

### DESCRIPTION

    When the mudlib moves an object "A"  inside  another  object  "B",  the
    driver (the move_object() efunction) does the following:

    1.  if "A" is living, causes "A" to call the init() in "B"

    2.  causes each living object in the inventory of "B" to call init() in
        "A".  regardless of whether "A" is living or not.

    3.  if "A" is living, causes "A" to call the init() in each  object  in
        the inventory of "B".

    Note:  an  object  is considered to be living if enable_commands(3) has
    been called by that object.

    Typically,  the  init(4)  function  in  an  object  is  used  to   call
    add_action(3) for each command that the object offers.

### SEE ALSO

    reset(4), move_object(3), enable_commands(3), living(3), add_action(3)

