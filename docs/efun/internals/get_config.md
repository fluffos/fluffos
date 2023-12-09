---
layout: doc
title: internals / get_config
---
# get_config

### NAME

    get_config() - query various driver config settings

### SYNOPSIS

    string | int get_config( int );

### DESCRIPTION

    This efun is used to query the driver's various config settings.
    Please refer to the "runtime_config.h" include file for a list of currently
    recognized options.

### EXAMPLE

```c
get_config( __RC_HEARTBEAT_INTERVAL_MSEC__ );
get_config( __RC_CALL_OUT_ZERO_NEST_LEVEL__ );
```


### SEE ALSO

    set_config(), /include/runtime_config.h
