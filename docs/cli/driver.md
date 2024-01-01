---
layout: doc
title: cli / driver
---
# cli / driver

`driver` is the main binary produced from FluffOS Project.

## Usage

```bash
./driver [flags] config_file
```

## Flags

| Flag                     | Description                                                                                                               |
|--------------------------|---------------------------------------------------------------------------------------------------------------------------|
| `-f<flag>`               | After initialization, before opening network connections, driver will call `void flag(string)` function on master object. |
| `-d<category>`           | Enable debug logging, this flag can occur multiple times. This is same as calling `set_debug_level()` EFUN.               |
| `--tracing <trace_file>` | Enable LPC tracing                                                                                                        |

## Debug logging Categories

| Category        | Description                          |
|-----------------|--------------------------------------|
| `call_out`      | Callouts                             |
| `d_flag`        | Legacy combination of various events |
| `connections`   | external connections                 |
| `mapping`       | Mapping operations                   |
| `sockets`       | LPC Sockets related logs             |
| `comp_func_tab` | Compilation                          |
| `LPC`           | LPC execution                        |
| `LPC_line`      | LPC execution                        |
| `event`         | Events                               |
| `dns`           | DNS resolution logs                  |
| `file`          | Filesystem operations                |
| `add_action`    | add_action packages related logs     |
| `telnet`        | Telnet protocol                      |
| `websocket`     | Websocket protocol                   |