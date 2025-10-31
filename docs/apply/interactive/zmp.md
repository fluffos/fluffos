---
layout: doc
title: interactive / zmp
---
# zmp_command

### NAME

    zmp_command - handle ZMP commands from client

### SYNOPSIS

    void zmp_command(string cmd, string *args) ;

### DESCRIPTION

    This apply is called when the driver receives a ZMP (Zenith Mud Protocol)
    command from a ZMP-capable client.

    **Arguments:**
    - `cmd`: The ZMP command name (e.g., "zmp.ping", "zmp.test")
    - `args`: Array of string arguments accompanying the command

    ZMP is a telnet protocol extension that allows structured communication
    between the client and server using null-terminated string arrays.

    Common ZMP commands include:
    - `zmp.ping` - Connection keepalive/test
    - `zmp.input` - Enhanced input handling
    - `zmp.time` - Time synchronization

    ZMP support must be enabled in the driver configuration with:
    ```
    enable zmp : 1
    ```

### EXAMPLE

    ```c
    void zmp_command(string cmd, string *args) {
        switch(cmd) {
            case "zmp.ping":
                // Respond to ping
                send_zmp("zmp.ping", ({}));
                break;
            case "zmp.test":
                // Handle test command with args
                write("ZMP test received with " + sizeof(args) + " arguments\n");
                break;
        }
    }
    ```

### SEE ALSO

    has_zmp(3), send_zmp(3)
