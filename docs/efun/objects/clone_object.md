---
layout: default
title: objects / clone_object
---

### NAME

    clone_object() - load a copy of an object

### SYNOPSIS

    object clone_object( string name );

### DESCRIPTION

    Load  a  new  object  from  definition <name>, and give it a new unique
    name.  Returns the new object.  An object with a nonzero  environment()
    cannot be cloned.

### SEE ALSO

    destruct(3), move_object(3), new(3)

