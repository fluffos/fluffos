---
layout: default
title: objects / tell_object
---

### NAME

    tell_object() - send a message to an object

### SYNOPSIS

    void tell_object( object ob, string str );

### DESCRIPTION

    Send  a message 'str' to object 'ob'. If it is an interactive object (a
    player), then the message will go to him, otherwise it will go  to  the
    local function "catch_tell()".

### SEE ALSO

    message(3), write(3), shout(3), say(3), tell_room(3)

