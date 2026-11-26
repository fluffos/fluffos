---
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

    It runs first, before the object's contents are moved out and before
    anything about the object is torn down, so the object is still fully
    usable here. Calling destruct() on the object from inside on_destruct()
    does nothing: the destruct already in progress finishes it. A shadowed
    object is notified once, after its shadows have been destructed.

    If a later step of the destruct fails before anything has been torn
    down (a content's move_or_destruct() raising an error, or a master or
    simul_efun object failing to reload), the destruct is abandoned and the
    object stays as it was; on_destruct() will run again on the next
    attempt.
