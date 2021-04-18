---
layout: default
title: objects / clone_object
---

### NAME

    clone_object() - load a copy of an object

### SYNOPSIS

    object clone_object( string name, ... );

### DESCRIPTION

    Load  a  new  object  from  definition <name>, and give it a new unique
    name.  Returns the new object.  An object with a nonzero  environment()
    cannot be cloned.  Optionally, additional arguments may be passed to
    clone_object() which will be passed to 'create()' in the new object.

### EXAMPLE

    object ob = clone_object("/obj/torch") ; // clone a torch object
    object money = clone_object("/obj/money", 10, "dollars" ) ; // clone a money object and set initial values

### SEE ALSO

    destruct(3), move_object(3), new(3)

