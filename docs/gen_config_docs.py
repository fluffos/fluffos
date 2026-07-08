#!/usr/bin/env python3
"""Generate docs/driver/config.md from the runtime-config tables in rc.cc.

The driver's recognized runtime config options live in two tables in
src/base/internal/rc.cc:

  * INT_FLAGS[]  -- integer options (name, default, min, max, category, prose)
  * STR_FLAGS[]  -- simple string options (name, required, category, prose)

Those tables are the single source of truth for both the parser AND this
document, so the docs cannot drift from the driver as long as this script is
re-run after editing them.

Usage:
    python3 docs/gen_config_docs.py            # regenerate docs/driver/config.md
    python3 docs/gen_config_docs.py --check     # exit 1 if the doc is stale (CI)

A handful of options with irregular parsing (the external ports, external
commands, the global include file, and the default fail message) are not in the
tables; they are documented from the hand-maintained SPECIAL_OPTIONS section
below. When you add or change one of those in rc.cc, update SPECIAL_OPTIONS too.
"""

import argparse
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
RC_CC = os.path.join(ROOT, "src", "base", "internal", "rc.cc")
OPTIONS_H = os.path.join(ROOT, "src", "base", "internal", "options_internal.h")
OUTPUT = os.path.join(HERE, "driver", "config.md")

# Order in which categories are emitted. Categories found in the tables but not
# listed here are appended at the end (with a warning) so nothing is dropped.
CATEGORY_ORDER = [
    "Identity & Network",
    "Directory Structure",
    "Core Files",
    "Logging",
    "Error Handling",
    "Timing & Lifecycle",
    "Limits",
    "Hash Tables",
    "Reset Behavior",
    "Language Behavior",
    "Type Checking",
    "Player I/O",
    "Diagnostics",
    "Performance",
    "Protocol Support",
    "Security",
]

# Built-in integer constants the table defaults may reference.
BUILTIN_CONSTS = {
    "INT_MAX": 2147483647,
    "UINT8_MAX": 255,
}


# --------------------------------------------------------------------------- #
# Minimal C++ initializer parsing
# --------------------------------------------------------------------------- #
def _extract_array_body(text, decl):
    """Return the text between the outermost braces of `<decl> = { ... }`."""
    marker = decl + "[] = {"
    start = text.find(marker)
    if start < 0:
        raise SystemExit(f"could not find `{marker}` in {RC_CC}")
    i = start + len(marker)
    depth, in_str, esc = 1, False, False
    body = []
    while i < len(text):
        ch = text[i]
        if in_str:
            body.append(ch)
            if esc:
                esc = False
            elif ch == "\\":
                esc = True
            elif ch == '"':
                in_str = False
        elif ch == '"':
            in_str = True
            body.append(ch)
        elif ch == "{":
            depth += 1
            body.append(ch)
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return "".join(body)
            body.append(ch)
        else:
            body.append(ch)
        i += 1
    raise SystemExit(f"unterminated `{decl}` table in {RC_CC}")


def _split_top_level_braces(body):
    """Yield the inner text of each top-level `{ ... }` group in `body`."""
    depth, in_str, esc, cur = 0, False, False, []
    for ch in body:
        if in_str:
            cur.append(ch)
            if esc:
                esc = False
            elif ch == "\\":
                esc = True
            elif ch == '"':
                in_str = False
            continue
        if ch == '"':
            in_str = True
            cur.append(ch)
            continue
        if ch == "{":
            depth += 1
            if depth == 1:
                cur = []
                continue
        if ch == "}":
            depth -= 1
            if depth == 0:
                yield "".join(cur)
                continue
        if depth >= 1:
            cur.append(ch)


def _split_fields(entry):
    """Split one initializer's comma-separated fields (quote-aware)."""
    fields, cur, in_str, esc = [], [], False, False
    for ch in entry:
        if in_str:
            cur.append(ch)
            if esc:
                esc = False
            elif ch == "\\":
                esc = True
            elif ch == '"':
                in_str = False
            continue
        if ch == '"':
            in_str = True
            cur.append(ch)
            continue
        if ch == ",":
            fields.append("".join(cur).strip())
            cur = []
            continue
        cur.append(ch)
    tail = "".join(cur).strip()
    if tail:
        fields.append(tail)
    return fields


def _unquote(field):
    field = field.strip()
    if field.startswith('"') and field.endswith('"'):
        field = field[1:-1]
    return field.replace('\\"', '"').replace("\\\\", "\\")


# --------------------------------------------------------------------------- #
# Default-value resolution
# --------------------------------------------------------------------------- #
def _load_cfg_consts():
    consts = dict(BUILTIN_CONSTS)
    with open(OPTIONS_H, encoding="utf-8") as fh:
        for line in fh:
            parts = line.split()
            if len(parts) >= 3 and parts[0] == "#define" and parts[2].lstrip("-").isdigit():
                consts[parts[1]] = int(parts[2])
    return consts


def _resolve_int(expr, consts):
    """Evaluate a simple integer expression (literals, shifts, named consts)."""
    expr = expr.strip()
    try:
        # Restricted: no builtins, names limited to resolved config constants.
        return int(eval(expr, {"__builtins__": {}}, consts))  # noqa: S307
    except Exception:
        return None


# --------------------------------------------------------------------------- #
# Option model
# --------------------------------------------------------------------------- #
REQUIRED_LABELS = {
    "kMustHave": "required",
    "kOptional": "optional",
    "kWarnMissing": "recommended",
}


def parse_options(text, consts):
    options = []

    for entry in _split_top_level_braces(_extract_array_body(text, "INT_FLAGS")):
        f = _split_fields(entry)
        if len(f) != 7:
            raise SystemExit(f"INT_FLAGS entry has {len(f)} fields, expected 7:\n  {entry}")
        key, _pos, default, low, high, category, desc = f
        options.append({
            "key": _unquote(key),
            "type": "int",
            "default": _resolve_int(default, consts),
            "default_src": default.strip(),
            "min": _resolve_int(low, consts),
            "max": _resolve_int(high, consts),
            "category": _unquote(category),
            "description": _unquote(desc),
            "required": None,
        })

    for entry in _split_top_level_braces(_extract_array_body(text, "STR_FLAGS")):
        f = _split_fields(entry)
        if len(f) != 6:
            raise SystemExit(f"STR_FLAGS entry has {len(f)} fields, expected 6:\n  {entry}")
        key, _pos, required, _tag, category, desc = f
        options.append({
            "key": _unquote(key),
            "type": "string",
            "default": None,
            "default_src": None,
            "min": None,
            "max": None,
            "category": _unquote(category),
            "description": _unquote(desc),
            "required": REQUIRED_LABELS.get(required.strip(), required.strip()),
        })

    return options


# --------------------------------------------------------------------------- #
# Rendering
# --------------------------------------------------------------------------- #
HEADER = """\
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
"""

FOOTER = """\
## See Also

- [driver](../cli/driver.md) - Driver command-line options
- [get_config](../efun/internals/get_config.md) - Query a config value at runtime
- [set_config](../efun/internals/set_config.md) - Modify a config value at runtime

## Reference Files

- `src/base/internal/rc.cc` - The `INT_FLAGS[]` / `STR_FLAGS[]` tables (source of truth)
- `src/Config.example` - Annotated example configuration
- `src/include/runtime_config.h` - Config slot constants
"""

# Options parsed with irregular/dynamic handling in rc.cc, documented by hand.
# Keep in sync with read_config() in src/base/internal/rc.cc.
SPECIAL_OPTIONS = """\
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
"""


def _range_note(opt):
    low, high = opt["min"], opt["max"]
    if low is None or high is None:
        return ""
    if low == 0 and high == BUILTIN_CONSTS["INT_MAX"]:
        return ""
    if high == BUILTIN_CONSTS["INT_MAX"]:
        return f" _(min {low})_"
    if low == 0:
        return f" _(max {high})_"
    return f" _(range {low}-{high})_"


def _description_cell(opt):
    desc = opt["description"]
    if opt["type"] == "string" and opt["required"] in ("required", "recommended"):
        desc = f"**{opt['required'].capitalize()}.** {desc}"
    desc += _range_note(opt)
    return desc.replace("|", "\\|")


def _default_cell(opt):
    if opt["type"] != "int":
        return "—"
    if opt["default"] is None:
        return f"`{opt['default_src']}`"
    return str(opt["default"])


def render(options):
    by_category = {}
    for opt in options:
        by_category.setdefault(opt["category"], []).append(opt)

    ordered = [c for c in CATEGORY_ORDER if c in by_category]
    extra = [c for c in by_category if c not in CATEGORY_ORDER]
    if extra:
        print(f"warning: categories not in CATEGORY_ORDER: {', '.join(sorted(extra))}",
              file=sys.stderr)
    ordered += sorted(extra)

    parts = [HEADER, "## Options\n"]
    for category in ordered:
        parts.append(f"### {category}\n")
        parts.append("| Setting | Type | Default | Description |")
        parts.append("|---------|------|---------|-------------|")
        for opt in by_category[category]:
            parts.append(
                f"| `{opt['key']}` | {opt['type']} | {_default_cell(opt)} | {_description_cell(opt)} |"
            )
        parts.append("")

    parts.append(SPECIAL_OPTIONS)
    parts.append(FOOTER)
    return "\n".join(parts).rstrip() + "\n"


# --------------------------------------------------------------------------- #
# Entry point
# --------------------------------------------------------------------------- #
def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--check", action="store_true",
                    help="exit non-zero if docs/driver/config.md is out of date")
    args = ap.parse_args()

    with open(RC_CC, encoding="utf-8") as fh:
        rc_text = fh.read()
    consts = _load_cfg_consts()
    options = parse_options(rc_text, consts)
    generated = render(options)

    if args.check:
        try:
            with open(OUTPUT, encoding="utf-8") as fh:
                current = fh.read()
        except FileNotFoundError:
            current = None
        if current != generated:
            print("docs/driver/config.md is out of date.", file=sys.stderr)
            print("Regenerate it with: python3 docs/gen_config_docs.py", file=sys.stderr)
            return 1
        print("docs/driver/config.md is up to date.")
        return 0

    with open(OUTPUT, "w", encoding="utf-8") as fh:
        fh.write(generated)
    print(f"wrote {os.path.relpath(OUTPUT, ROOT)} ({len(options)} table options)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
