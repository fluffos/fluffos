---
layout: doc
title: cli / symbol
---
# cli / symbol

`symbol` is a utility tool for loading and analyzing LPC files using the FluffOS driver infrastructure.

## Usage

```bash
./symbol <config> lpc_file
```

## Arguments

| Argument     | Description                                           |
|--------------|-------------------------------------------------------|
| `config`     | Path to the driver configuration file                 |
| `lpc_file`   | Path to the LPC file to load and process              |

## Description

The `symbol` tool initializes the FluffOS driver with the specified configuration file and attempts to load the specified LPC file. It enables symbol processing and reports whether the file was successfully loaded.

This tool is useful for:
- Validating LPC file syntax
- Testing object compilation outside of the full driver environment
- Debugging compilation issues

## Exit Codes

| Code | Description                                  |
|------|----------------------------------------------|
| `0`  | Success - LPC file loaded successfully       |
| `1`  | Error - Failed to load object or bad usage   |

## Example

```bash
./symbol etc/config.test /domains/test/obj/sword.c
```

## See Also

- [driver](driver.md) - Main FluffOS driver executable
- [lpcc](lpcc.md) - LPC compiler utility
