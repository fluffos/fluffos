---
layout: default
title: objects / first_inventory
---

### NAME

    first_inventory() - return the first item in an object's inventory

### SYNOPSIS

    object first_inventory( mixed ob );

### DESCRIPTION

    Return  the first object in the inventory of <ob>, where <ob> is either
    an object or the file name of an object.

    If no object is supplied, this efun will default to this_object().

### SEE ALSO

    file_name(3), next_inventory(3), all_inventory(3), deep_inventory(3)

