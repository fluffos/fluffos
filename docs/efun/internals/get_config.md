---
layout: default
title: internals / get_config
---

### NAME

    get_config() - query various driver config settings

### SYNOPSIS

    string | int get_config( int option );

### DESCRIPTION

    This efun is used to query the driver's various config settings.
    Please refer to the "runtime_config.h" include file for a list of
    currently recognized options.

### SEE ALSO

    set_config(), /include/runtime_config.h
