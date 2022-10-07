---
layout: default
title: contrib / event.pre
---

### NAME

    event() - call event_* in other objects with specified parameters. similar
    to call_other, but with no return type.

### SYNOPSIS

    void event(object | object * target, string event_name, mixed *args..);

### DESCRIPTION

    Calls "event_" + event_name in target. 

    Target can be a single object or an array of objects.

    If the target is a single object, the event efun will first call the event
    function in the object and then it will call the event function all of the
    objects in the target's all_inventory().

    If the target is an array of objects, the event efun will call the event
    function in all of the objects specified. Unlike the single-target version,
    it will not cascade to each of the object's inventories.

    The efun will pass as its first argument the calling object followed by
    all of the specified arguments.


### EXAMPLE

    // In a room
    
    // will call event_heal_up on the room, and then every object in the room
    event( this_object(), "heal_up", 50, 50 ) ; 

    // will call event_heal_up in the results of users() efun
    event( users(), "heal_up", 25, 25 ) ;

    // In a player object

    // To take advantage of this event, write an event_heal_up in the player
    // object to receive the call
    void event_heal_up(object source, int health, int mana)
    {
        message("heal", sprintf("You were healed %d health and %d mana by %O.\n", health, mana, source), this_object()) ;
    }
