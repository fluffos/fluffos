---
title: contrib / set_os_env
---
# set_os_env

### NAME

    set_os_env - set or unset an OS environment variable (allow-listed)

### SYNOPSIS

    int set_os_env( string name, string value );
    int set_os_env( string name );

### DESCRIPTION

    Sets the operating-system environment variable 'name' to 'value', or
    unsets it when 'value' is omitted. Returns 1 on success, 0 on an
    operating-system level failure.

    Writes are denied unless 'name' appears in the runtime config option
    "writable os environment variables" (colon-separated, empty by
    default). Writable names are implicitly readable via get_os_env().

### EXAMPLES

    ```c
    set_os_env("MUD_STATE", "booting");
    set_os_env("MUD_STATE");   // unset
    ```

### SEE ALSO

    get_os_env(3)
