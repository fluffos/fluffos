---
title: contrib / get_os_env
---
# get_os_env

### NAME

    get_os_env - read an OS environment variable (allow-listed)

### SYNOPSIS

    string get_os_env( string name );

### DESCRIPTION

    Returns the value of the operating-system environment variable 'name',
    or undefined if the variable is not set.

    Access is denied unless 'name' appears in the runtime config option
    "allowed os environment variables" (or in "writable os environment
    variables", which implies readability). Both lists are colon-separated
    and empty by default, so no variable is accessible until the host
    administrator opts in.

### EXAMPLES

    ```c
    string path = get_os_env("PATH");
    ```

### SEE ALSO

    set_os_env(3)
