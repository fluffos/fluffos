---
layout: default
title: internals / set_config
---

### NAME

    set_config() - query various driver config settings

### SYNOPSIS

    void set_config( int option, mixed value );

### DESCRIPTION

    This efun is used to set the driver's various config settings during
    runtime. Please refer to the "runtime_config.h" include file for a list of
    currently recognized options.

    Note that this only modifies the currently running options and will not
    persist across driver restarts.

### SEE ALSO

    /include/runtime_config.h
