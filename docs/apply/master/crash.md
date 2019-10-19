---
layout: default
title: master / crash
---

### NAME

    crash  -  function  in  master  that  is called in the event the driver
    crashes

### SYNOPSIS

    void crash( string crash_message, object command_giver,
                object current_object );

### DESCRIPTION

    The driver calls crash() in master in the event that the driver crashes
    (segmentation  fault,  bus  error, etc).  This function offers a way to
    shutdown the mudlib (safe players and other important data) before  the
    driver  crashes.   It also lets you log various useful information such
    as what signal crashed the driver, what object was active, who the cur‐
    rent player was etc.

### SEE ALSO

    shutdown(3)

