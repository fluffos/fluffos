---
layout: default
title: objects / deep_inventory
---

### NAME

    deep_inventory() - return the nested inventory of an object

### SYNOPSIS

    object *deep_inventory( object ob );

### DESCRIPTION

    Returns  an array of the objects contained in the inventory of <ob> and
    all the objects contained in the inventories of those  objects  and  so
    on.

### SEE ALSO

    first_inventory(3), next_inventory(3), all_inventory(3)

