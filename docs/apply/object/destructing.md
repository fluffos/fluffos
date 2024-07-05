---
layout: doc
title: object / destructing
---
# clean_up

### NAME

    destructing - notification that the object is being destructed

### SYNOPSIS

    void destructing(void)

### DESCRIPTION

    The destructing() function is called by the driver when the object is
    scheduled for destruction. This function is called before the object is
    actually destructed, and is intended to allow the object to perform any
    necessary cleanup before it is removed from the game.
