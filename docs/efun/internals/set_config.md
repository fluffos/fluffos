---
layout: doc
title: internals / set_config
---
# set_config

### NAME

    set_config() - set various driver config settings during runtime

### SYNOPSIS

    void set_config( int option, mixed value );

### DESCRIPTION

    This efun is used to set the driver's various config settings during
    runtime. Please refer to the "runtime_config.h" include file for a list of
    currently recognized options.

    Note that this only modifies the currently running options and will not
    persist across driver restarts.

### EXAMPLE

```c
// set the driver's heartbeat to 1000 milliseconds
set_config(__RC_HEARTBEAT_INTERVAL_MSEC__, 1000)

// adjust the driver's limit for call_out(0)
set_config(__RC_CALL_OUT_ZERO_NEST_LEVEL__, 250)
```

### SEE ALSO

    get_config(), /include/runtime_config.h
