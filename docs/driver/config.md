---
layout: doc
title: driver / config
---
# Driver Configuration File

FluffOS uses a runtime configuration file to specify various settings for the MUD server. This file is passed as an argument when starting the driver.

## Basic Usage

```bash
./driver path/to/config.cfg
```

## File Format

- Lines beginning with `#` are comments
- Empty lines are ignored
- Format: `setting_name : value`
- Paths are relative to the mudlib directory (except mudlib directory itself)

## Core Settings

### Identity & Network

| Setting | Description | Example |
|---------|-------------|---------|
| `name` | Name of your MUD | `name : My MUD` |
| `mud ip` | IP address to bind to (0.0.0.0 for all) | `mud ip : 0.0.0.0` |
| `port number` | Primary telnet port | `port number : 4000` |

### External Ports

FluffOS supports multiple external ports with different protocols:

```
# Basic telnet port
external_port_1: telnet 5000

# WebSocket port
external_port_2: websocket 8080

# WebSocket with TLS
external_port_3: websocket 8443
external_port_3_tls: cert=path/to/cert.pem key=path/to/key.pem

# Telnet with TLS
external_port_4: telnet 4443
external_port_4_tls: cert=path/to/cert.pem key=path/to/key.pem
```

**WebSocket Support:**
```
websocket http dir : path/to/www
```
Specifies the directory containing the web client HTML/JS files.

### Directory Structure

| Setting | Description | Example |
|---------|-------------|---------|
| `mudlib directory` | Absolute path to mudlib | `mudlib directory : /home/mud/lib` |
| `log directory` | Where to store logs | `log directory : /log` |
| `include directories` | Paths for #include <> | `include directories : /include:/sys` |

### Core Files

| Setting | Description | Example |
|---------|-------------|---------|
| `master file` | Path to master object | `master file : /single/master` |
| `simulated efun file` | Path to simul_efun object | `simulated efun file : /single/simul_efun` |
| `global include file` | Auto-included in all objects | `global include file : <globals.h>` |

### Logging

| Setting | Description | Example |
|---------|-------------|---------|
| `debug log file` | Debug output filename | `debug log file : debug.log` |
| `log commands` | Log all player commands | `log commands : 0` |

### Error Handling

| Setting | Description | Example |
|---------|-------------|---------|
| `default error message` | Message when no action matches | `default error message : What?` |
| `default fail message` | Default notify_fail message | `default fail message : What?` |

### Memory Management

| Setting | Description | Example |
|---------|-------------|---------|
| `time to clean up` | Seconds before cleanup | `time to clean up : 600` |
| `time to reset` | Seconds between resets | `time to reset : 1800` |
| `time to swap` | Seconds before swap | `time to swap : 300` |
| `evaluator stack size` | Max function call depth | `evaluator stack size : 65536` |
| `inherit chain size` | Max inheritance depth | `inherit chain size : 30` |

### Performance & Limits

| Setting | Description | Example |
|---------|-------------|---------|
| `maximum array size` | Max array elements | `maximum array size : 15000` |
| `maximum buffer size` | Max buffer size | `maximum buffer size : 400000` |
| `maximum mapping size` | Max mapping size | `maximum mapping size : 150000` |
| `maximum string length` | Max string length | `maximum string length : 200000` |
| `maximum bits in a bitfield` | Max bitfield size | `maximum bits in a bitfield : 12000` |
| `maximum byte transfer` | Max bytes per read/write | `maximum byte transfer : 200000` |
| `maximum read file size` | Max file read size | `maximum read file size : 200000` |

### Execution Limits

| Setting | Description | Example |
|---------|-------------|---------|
| `max eval cost` | Max ticks per execution | `max eval cost : 5000000` |
| `max local variables` | Max local vars per function | `max local variables : 30` |
| `max call depth` | Max nested calls | `max call depth : 150` |

### Protocol Support

| Setting | Description | Example |
|---------|-------------|---------|
| `enable mxp` | Enable MXP protocol | `enable mxp : 1` |
| `enable gmcp` | Enable GMCP protocol | `enable gmcp : 1` |
| `enable zmp` | Enable ZMP protocol | `enable zmp : 1` |
| `enable mssp` | Enable MSSP protocol | `enable mssp : 1` |
| `enable msp` | Enable MSP protocol | `enable msp : 1` |

### Compression

| Setting | Description | Example |
|---------|-------------|---------|
| `compress protocol` | Enable MCCP2 | `compress protocol : 1` |

### Security

| Setting | Description | Example |
|---------|-------------|---------|
| `valid bind` | Check valid_bind() apply | `valid bind : 1` |
| `valid override` | Check valid_override() apply | `valid override : 1` |

## Example Configuration

```
###############################################################################
#                     FluffOS Configuration Example                          #
###############################################################################

# MUD Identity
name : MyMUD
mud ip : 0.0.0.0
port number : 4000

# Additional Ports
external_port_1: websocket 8080
external_port_2: telnet 5000

# WebSocket Client
websocket http dir : www

# Directory Structure
mudlib directory : /home/mud/lib
log directory : /log
include directories : /include:/sys

# Core Files
master file : /single/master
simulated efun file : /single/simul_efun
global include file : <globals.h>

# Logging
debug log file : debug.log

# Memory & Performance
time to clean up : 600
time to reset : 1800
max eval cost : 5000000
evaluator stack size : 65536

# Protocol Support
enable gmcp : 1
enable mxp : 1
compress protocol : 1

# Limits
maximum array size : 15000
maximum string length : 200000
maximum mapping size : 150000
```

## Tips

**Development Config:**
- Lower limits for faster testing
- Enable debug logging
- Shorter cleanup/reset times

**Production Config:**
- Higher limits for performance
- Minimal logging
- Enable compression
- Configure TLS for security

**WebSocket Setup:**
```
# Serve web client on port 8080
external_port_1: websocket 8080
websocket http dir : www

# Place your index.html in: mudlib/../www/
```

**TLS Certificate Setup:**
```
# Generate self-signed cert (development only)
openssl req -x509 -newkey rsa:4096 -keyout key.pem \
  -out cert.pem -days 365 -nodes

# Use in config
external_port_1: telnet 4443
external_port_1_tls: cert=etc/cert.pem key=etc/key.pem
```

## See Also

- [driver](../cli/driver.md) - Driver command-line options
- [get_config](../efun/internals/get_config.md) - Query config at runtime
- [set_config](../efun/internals/set_config.md) - Modify config at runtime

## Reference Files

- Example config: `testsuite/etc/config.test` in the source
- All options: See `src/include/runtime_config.h`
