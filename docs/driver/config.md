---
layout: doc
title: driver / config
---
<!-- ===========================================================================
     AUTO-GENERATED FILE -- DO NOT EDIT BY HAND.

     This page is generated from the INT_FLAGS[] and STR_FLAGS[] tables in
     src/base/internal/rc.cc by docs/gen_config_docs.py. To change an option's
     documentation, edit its `category`/`description` in rc.cc and run:

         python3 docs/gen_config_docs.py

     CI (.github/workflows/config-docs.yml) verifies this file is up to date.
=========================================================================== -->
# Driver Configuration File

FluffOS reads a runtime configuration file at startup to configure the driver.
The file is passed as the first argument to the `driver` executable.

```bash
./driver path/to/config.cfg
```

## File Format

- Lines beginning with `#` are comments; blank lines are ignored.
- One setting per line, in the form `setting name : value`.
- Most mudlib paths are relative to the mudlib directory; exceptions are noted
  per option (e.g. `mudlib directory` is an absolute OS path, and `log directory`
  is a filesystem path relative to the driver's working directory).
- Integer options out of range are reset to their default with a warning.

For extended commentary and example values, see the annotated `src/Config.example`
in the source tree. The tables below are generated directly from the driver, so
they always match the options it actually recognizes.

## Options

### Identity & Network

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `name` | string | — | **Required.** Name of this MUD. |
| `mud ip` | string | — | IP address to bind to; useful on hosts with multiple network addresses. |

### Directory Structure

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `mudlib directory` | string | — | **Required.** Absolute path to the mudlib root (this path is not relative to the mudlib). |
| `log directory` | string | — | **Required.** Filesystem directory for debug.log and stats files, resolved relative to the driver's working directory (leading slashes are stripped); not a mudlib virtual path. |
| `include directories` | string | — | **Required.** Colon-separated list of directories searched by `#include <...>`. |

### Core Files

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `master file` | string | — | **Required.** Path to the object that defines the master object. |
| `simulated efun file` | string | — | Path to the object that defines global simulated efuns. |

### Logging

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `debug log file` | string | — | **Recommended.** Filename (within the log directory) for the driver's debug log. |

### Error Handling

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `mudlib error handler` | int | 1 | Pass runtime errors to the master object's error_handler() instead of handling them in the driver. |
| `trap crashes` | int | 1 | Call crash() in the master object and shut down cleanly on signals that would otherwise crash the driver. |
| `default error message` | string | — | Message shown to players when error() occurs. |

### Timing & Lifecycle

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `time to clean up` | int | 600 | Seconds an object may be idle before clean_up() is called on it; should be well above 'time to swap'. |
| `time to reset` | int | 900 | Seconds between successive reset() calls on an object. |
| `time to swap` | int | 300 | Seconds an unused object stays in memory before being swapped out; 0 disables swapping. |
| `gametick msec` | int | 1000 | Granularity of in-game time in milliseconds (the shortest visible time interval). |
| `heartbeat interval msec` | int | 1000 | Heartbeat interval in milliseconds. |

### Limits

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `evaluator stack size` | int | 65536 | Maximum size of the evaluator stack, which holds all local variables and call arguments. |
| `inherit chain size` | int | 30 | Maximum depth of an object's inheritance chain. |
| `maximum evaluation cost` | int | 30000000 | Maximum eval cost a single thread may consume before execution is aborted. |
| `maximum local variables` | int | 64 | Maximum number of local variables in a single function. _(range 64-255)_ |
| `maximum call depth` | int | 150 | Maximum nesting depth of LPC function calls. |
| `maximum array size` | int | 15000 | Maximum number of elements in a single array. |
| `maximum buffer size` | int | 1048576 | Maximum size, in bytes, of a single buffer variable. |
| `maximum mapping size` | int | 150000 | Maximum number of entries in a single mapping. |
| `maximum string length` | int | 1048576 | Maximum length, in bytes, of a single string variable. |
| `maximum bits in a bitfield` | int | 12000 | Maximum number of bits in a bitfield (stored 6 bits per printable byte). |
| `maximum byte transfer` | int | 262144 | Maximum number of bytes a single read_bytes()/write_bytes() call may transfer. |
| `maximum read file size` | int | 262144 | Maximum size, in bytes, of a file that read_file() may read. |

### Hash Tables

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `hash table size` | int | 65536 | Size of the shared-string hash table; should be prime, roughly 1/5 of the number of distinct strings. _(min 7001)_ |
| `object table size` | int | 4096 | Size of the object hash table; roughly 1/4 of the number of objects in the game. _(min 1024)_ |
| `living hash table size` | int | 256 | Size of the find_living() hash table; must be one of 4, 16, 64, 256, 1024, or 4096. _(min 256)_ |

### Reset Behavior

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `no resets` | int | 0 | Completely disable the periodic calling of reset(). |
| `lazy resets` | int | 0 | Only call reset() when an object is touched via call_other() or move_object(). |
| `randomized resets` | int | 1 | Spread reset() calls over a randomized interval rather than firing them all at once. |

### Language Behavior

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `sane explode string` | int | 1 | explode() strips at most one leading delimiter (and still one trailing delimiter). |
| `reversible explode string` | int | 0 | Make implode(explode(x, y), y) always equal x; overrides 'sane explode string'. |
| `sane sorting` | int | 1 | Use a well-defined, stable ordering for the driver's sorting operations. |
| `wombles` | int | 0 | Disallow spaces between the start/end token characters of arrays, mappings, and functionals. |
| `this_player in call_out` | int | 1 | Make this_player() usable from within call_out() callbacks. |
| `reverse defer` | int | 0 | Run deferred functions registered with defer() in reverse order. |
| `old range behavior` | int | 0 | Treat negative range indices in strings/buffers as counting from the end (rvalue use only). |
| `warn old range behavior` | int | 1 | Warn when code relies on 'old range behavior'. |
| `enable_commands call init` | int | 1 | Call init() in an object when enable_commands() is invoked on it. |
| `sprintf add_justified ignore ANSI colors` | int | 1 | Make sprintf() column justification ignore ANSI color codes when computing field width. |
| `call_out(0) nest level` | int | 1000 | Maximum nesting level for chains of call_out(0) within a single backend cycle. |

### Type Checking

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `call other type check` | int | 0 | Enable type checking for call_other() (the -> operator on objects). |
| `call other warn` | int | 0 | Emit warnings instead of errors for call_other() type mismatches. |
| `old type behavior` | int | 0 | Reintroduce a legacy type-checking bug for backwards compatibility. |

### Player I/O

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `no ansi` | int | 1 | Replace ANSI escape characters (ASCII 27) in user input with a space before add_actions run. |
| `strip before process input` | int | 1 | Strip ANSI before process_input() sees the input, rather than only before add_actions are called. |
| `interactive catch tell` | int | 0 | Call catch_tell() on interactive users as well as on NPCs. |
| `receive snoop` | int | 1 | Send snoop text to receive_snoop() in the snooper instead of directly via add_message(). |
| `snoop shadowed` | int | 0 | Report snooped output even when the target's catch_tell() is shadowed (prefixed with $$). |
| `noninteractive stderr write` | int | 0 | Write tells/messages sent to non-interactive objects to stderr, prefixed with ']' (legacy behavior). |

### Diagnostics

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `warn tab` | int | 0 | Warn when source files are indented with tabs instead of spaces. |
| `trace` | int | 1 | Enable the trace() and traceprefix() efuns (leaving it off runs slightly faster). |
| `trace code` | int | 0 | Include the preceding lines of LPC code in error traces (slower). |
| `has console` | int | 1 | Allow the driver's interactive console via the -C command-line argument. |
| `suppress argument warnings` | int | 1 | Suppress unused-argument warnings, warning only about unused local variables. |
| `trace lpc execution context` | int | 0 | Record LPC execution context for tracing and debugging. |
| `trace lpc instructions` | int | 0 | Trace individual LPC instructions for debugging. |

### Protocol Support

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `enable mxp` | int | 0 | Advertise and enable the MXP telnet protocol. |
| `enable gmcp` | int | 0 | Advertise and enable the GMCP telnet protocol. |
| `enable zmp` | int | 0 | Advertise and enable the ZMP telnet protocol. |
| `enable mssp` | int | 1 | Advertise and enable the MSSP telnet protocol. |
| `enable msp` | int | 1 | Advertise and enable the MSP telnet protocol. |
| `enable msdp` | int | 0 | Advertise and enable the MSDP telnet protocol. |

## Ports and Connections

The listening ports are configured with numbered `external_port_N` entries
(N = 1 to 5). Each names a protocol and a port number:

```
external_port_1 : telnet 4000
external_port_2 : binary 4001
external_port_3 : websocket 8080
```

Recognized protocols are `telnet`, `binary`, `ascii`, `MUD`, and `websocket`.

| Setting | Description |
|---------|-------------|
| `external_port_N` | Protocol and port for listener N, e.g. `telnet 4000`. |
| `external_port_N_tls` | Enable TLS on listener N: `cert=path/to/cert.pem key=path/to/key.pem`. |
| `websocket http dir` | Directory (under `src/www`) of static files served to web clients; required when a `websocket` port is defined. |
| `port number` | Legacy single telnet port; equivalent to defining `external_port_1 : telnet <n>`. |

A `websocket` port requires `websocket http dir` to be set. With TLS, point
`cert=`/`key=` at a PEM certificate and key, for example:

```
external_port_1 : telnet 4443
external_port_1_tls : cert=etc/cert.pem key=etc/key.pem
```

## External Commands

When the driver is built with `PACKAGE_EXTERNAL`, external programs callable via
`external_start()` are declared with numbered entries:

| Setting | Description |
|---------|-------------|
| `external_cmd_N` | Command line for external slot N (1-based). |

## Other Options

| Setting | Description |
|---------|-------------|
| `global include file` | Header automatically `#include`d in every compiled object, e.g. `"/include/globals.h"` or `<globals.h>`. Quotes are added if omitted. |
| `default fail message` | Message used when an action returns 0 and no `notify_fail()` was set. Defaults to `What?`. |

## Obsolete Options

These settings are no longer used and should be removed. The driver prints a
warning if any of these appear: `address server ip`, `address server port`,
`reserved size`, `fd6 kind`, `fd6 port`, `binary directory`, `swap file`.

These are accepted for backwards compatibility but silently ignored:
`maximum users`, `compiler stack size`.

## See Also

- [driver](../cli/driver.md) - Driver command-line options
- [get_config](../efun/internals/get_config.md) - Query a config value at runtime
- [set_config](../efun/internals/set_config.md) - Modify a config value at runtime

## Reference Files

- `src/base/internal/rc.cc` - The `INT_FLAGS[]` / `STR_FLAGS[]` tables (source of truth)
- `src/Config.example` - Annotated example configuration
- `src/include/runtime_config.h` - Config slot constants
