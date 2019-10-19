---
layout: default
title: objects / next_inventory
---

### NAME

    next_inventory() - return the next object in the same inventory

### SYNOPSIS

    object next_inventory( object ob );

### DESCRIPTION

    Return the next object in the same inventory as 'ob'.

    Warning:   If  the  object  'ob'  is  moved  by  "move_object()",  then
    "next_inventory()" will return an object from the new inventory.

### SEE ALSO

    first_inventory(3), all_inventory(3), deep_inventory(3)

