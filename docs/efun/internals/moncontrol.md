---
layout: default
title: internals / moncontrol
---

### NAME

    moncontrol() - turns on/off profiling during execution

### SYNOPSIS

    void moncontrol( int on );

### DESCRIPTION

    If passed 1, moncontrol() enables profiling.  If passed 0, moncontrol()
    disables profiling.  It can be called many times during execution, typ‐
    ical use is to profile only certain parts of driver execution.  moncon‐
    trol() has no effect if profiling is  not  enabled  at  driver  compile
    time.

### SEE ALSO

    opcprof(3), function_profile(3)

