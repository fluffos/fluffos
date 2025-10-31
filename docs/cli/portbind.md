---
layout: doc
title: cli / portbind
---
# cli / portbind

`portbind` is a privilege-separated port binding utility for FluffOS. It allows the driver to bind to privileged ports (< 1024) and then drop privileges before execution.

## Usage

```bash
./portbind -p <port> [-d <driver_path>] [-u <uid>] [-g <gid>] [-i <ip_address>] [driver_args...]
```

## Options

| Option            | Description                                                  | Required |
|-------------------|--------------------------------------------------------------|----------|
| `-p <port>`       | Port number to bind to                                       | Yes      |
| `-d <driver>`     | Path to the driver executable (default: `./driver`)          | No       |
| `-u <uid>`        | User ID to drop privileges to after binding                  | No       |
| `-g <gid>`        | Group ID to drop privileges to after binding                 | No       |
| `-i <ip_address>` | IP address to bind to (default: INADDR_ANY)                  | No       |
| `driver_args...`  | Additional arguments passed to the driver                    | No       |

## Description

`portbind` solves the problem of binding to privileged ports (ports below 1024) which typically require root privileges. The tool:

1. Binds a socket to the specified port (requires root if port < 1024)
2. Passes the bound socket to the driver via file descriptor 6
3. Drops privileges to the specified UID/GID if provided
4. Executes the driver with any additional arguments

This allows the driver to run as an unprivileged user while still binding to privileged ports like port 23 (telnet) or port 80 (HTTP).

## Security Considerations

- The `portbind` utility itself must have appropriate permissions (typically setuid root) to bind to privileged ports
- Always specify `-u` and `-g` to drop privileges after binding
- The driver will inherit the bound socket on file descriptor 6

## Examples

**Bind to port 23 (telnet) and run as user 1000:**
```bash
sudo ./portbind -p 23 -u 1000 -g 1000 -d ./driver etc/config.prod
```

**Bind to a specific IP and port:**
```bash
sudo ./portbind -p 80 -i 192.168.1.100 -u www-data -g www-data ./driver etc/config.web
```

**Bind to port 4000 (non-privileged, for testing):**
```bash
./portbind -p 4000 -d ./driver etc/config.test -ftest
```

## See Also

- [driver](driver.md) - Main FluffOS driver executable

## Notes

- The socket is placed on file descriptor 6 before executing the driver
- The driver configuration should be set up to use the pre-bound socket
- All arguments after the portbind options are passed to the driver unchanged
