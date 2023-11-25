---
layout: doc
title: object / move_or_destruct
---
# move_or_destruct

### NAME

    move_or_destruct - ask an object to move to the specified destination

### SYNOPSIS

    void move_or_destruct( object dest );

### DESCRIPTION

    When an object is destructed, prior to its destruction, this apply will be
    called on all objects in its inventory. 'dest' will be the environment of
    the object being destructed. target object of this apply does not move
    itself out of the object being destructed, it will be destructed as well.

### EXAMPLE

    For example, if you have a room, and in it an object (ob1) which contains
    another object (ob2), and a further object (ob3), and you destruct the
    room, then the following will happen:

    Deep Scanning: OBJ(/d/sable/sable/rooms/ww2) EUID(Sable) UID(Sable)
    1:  OBJ(/u/g/gesslar/ob1#1481) EUID(gesslar) UID(gesslar)
      1:  OBJ(/u/g/gesslar/ob2#1482) EUID(gesslar) UID(gesslar)
        1:  OBJ(/u/g/gesslar/ob3#1483) EUID(gesslar) UID(gesslar)

    2023/09/24 04:02:45 move_or_destruct() called on ob1.c, value of dest is 0
    2023/09/24 04:02:45 move_or_destruct() called on ob2.c, value of dest is /d/sable/sable/rooms/ww2
    2023/09/24 04:02:45 move_or_destruct() called on ob3.c, value of dest is /u/g/gesslar/ob1#1481

### SEE ALSO

    destruct(3), move_object(3), init(4)
