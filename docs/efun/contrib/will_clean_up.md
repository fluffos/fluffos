---
layout: default
title: contrib / will_clean_up
---

### NAME

    will_clean_up - reports whether the object specified is
    eligible to be cleaned up by the driver

### SYNOPSIS

    int will_clean_up(object ob)

### DESCRIPTION

    Returns 1 if

    1) clean_up() is defined in the object, and
    2) clean_up() has not yet been called on the object by the driver, or
       the last clean_up() call from the driver resulted in 1
    
    Otherwise returns 0
