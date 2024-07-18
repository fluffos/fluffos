---
layout: doc
title: object / on_destruct
---
# on_destruct

### NAME

    on_destruct - notification that the object is being destructed

### SYNOPSIS

    void on_destruct(void)

### DESCRIPTION

    The on_destruct() function is called by the driver when the object is
    scheduled for destruction. This function is called before the object is
    actually destructed, and is intended to allow the object to perform any
    necessary cleanup before it is removed from the game.
