---
layout: default
title: objects / present
---

### NAME

    present() - find an object by id

### SYNOPSIS

    object present( mixed str, object ob );

### DESCRIPTION

    if first argument is string, second argument is 0/missing, searches for
    a object that id(name) = 1 in this object inventory and  this  object's
    environment's inventory.  Returns the found object.

    if  first  argument is string, second argument is object, only searches
    for traget in that object's inventory.

    "foo 1" means the first "foo" in  the  inventory.   the  first  in  the
    inventory is the last object that moved in.

    if  first  argument  is  object, second argument is 0/missing, check if
    object is in this object's inventory, or as a sibling in this  object's
    evnironemnt's inventory, returns object's parent.  // FIXME: this looks
    like a mis-feature.

    if first argument is  object,  second  argument  is  object,  check  if
    object1 is in object2 inventory, return object1 if true.

    If  object  is  hidden (via set_hide()), and current object is not hid‐
    able.  returns 0

### SEE ALSO

    move_object(3), environment(3)

