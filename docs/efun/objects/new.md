---
layout: default
title: objects / new
---

### NAME

    new() - load a copy of an object

### SYNOPSIS

    object new( string name, ... );

### DESCRIPTION

    Load  a  new  object  from  definition <name>, and give it a new unique
    name.  Returns the new object.  An object with a nonzero  environment()
    cannot be cloned.  Optionally, additional arguments may be passed to
    new() which will be passed to 'create()' in the new object.

### EXAMPLE

    object ob = new("/obj/torch") ; // clone a torch object
    object money = new("/obj/money", 10, "dollars" ) ; // clone a money object and set initial values

### SEE ALSO

    clone_object(3), destruct(3), move_object(3)

