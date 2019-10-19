---
layout: default
title: interactive / add_action
---

### NAME

    add_action() - bind a command verb to a local function

### SYNOPSIS

    void add_action( string | function fun, string | string * cmd, int flag
    );

### DESCRIPTION

    Set up a local function <fun> to be called when user input matches  the
    command  <cmd>. Functions called by a player command will get the argu‐
    ments as a string. It must then return 0 if it was the  wrong  command,
    otherwise 1.

    If  the second argument is an array, then all the commands in the array
    will call the second function.  It is possible to find out  which  com‐
    mand called the function with query_verb().

    If  it  was  the  wrong command, the parser will continue searching for
    another command, until one  returns  true  or  give  error  message  to
    player.

    Usually  add_action() is called only from an init() routine. The object
    that defines commands must be present to the player, either  being  the
    player,  being carried by the player, being the room around the player,
    or being an object in the same room as the player.

    If argument <flag> is 1, then only the leading characters of  the  com‐
    mand  has  to  match  the verb <cmd> and the entire verb is returned by
    query_verb().  If argument <flag> is 2, then again,  only  the  leading
    characters must match, but query_verb() will only return the characters
    following <cmd>.

### SEE ALSO

    query_verb(3), remove_action(3), init(4)

