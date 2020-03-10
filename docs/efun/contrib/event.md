---
layout: default
title: contrib / event.pre
---

### NAME

    event

### SYNOPSIS

    void event(object | object *, string, ...);

### DESCRIPTION

    calls "event_" + string(...) in:

    object *:
    each given object

    object:
    object + (if not destructed) all_inventory(object)
