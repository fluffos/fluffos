---
title: WebSocket LPC Debugger
---
# WebSocket LPC Debugger

FluffOS can expose a source-level LPC debugger over a dedicated WebSocket
port, speaking the [Debug Adapter Protocol](https://microsoft.github.io/debug-adapter-protocol/)
(DAP) — the same protocol VS Code, and many other editors, use for native
language debuggers. When enabled, a debugger client can attach to a running
driver, set breakpoints in mudlib source files, single-step LPC execution,
and inspect (and edit) the call stack, local variables, and any loaded
object.

See `src/debugger/DESIGN.md` in the repository for the full architecture and
design rationale.

## Enabling the debugger

The debugger is fully disabled — no listener, no per-instruction overhead —
unless `debugger port` is set in your config file:

```
debugger port : 4711
```

By default the listener binds to `127.0.0.1` only. To accept connections
from another host, set `debugger password` (required for any non-loopback
`debugger address`) so an attaching client must present a shared secret:

```
debugger port : 4711
debugger address : 0.0.0.0
debugger password : change-me
```

See [Configuration](../../driver/config.md) for the full option reference
(category "Debugger").

## How it works

- The debugger listens on its own `libwebsockets` context, independent from
  the player-facing telnet/websocket ports, using the `dap` subprotocol.
- While the VM is paused at a breakpoint, the entire driver is paused with
  it (no `heart_beat`, `call_out`, or player command is processed) — this is
  a stop-the-world debugger, intended for development servers. If the
  debugger client disconnects while the VM is stopped, the driver
  automatically resumes so a dropped connection can never freeze the mud.
- Breakpoints are resolved against the compiler's per-program line tables;
  a breakpoint on a line with no code snaps to the next line that has one.
  Breakpoints on a file that isn't loaded yet stay pending and bind
  automatically once a matching program is compiled. A breakpoint can also
  carry a hit count condition (VS Code's "Edit Breakpoint" → "Hit Count"):
  an optional comparator (`> >= < <= == !=` or `%`) followed by a number,
  e.g. `>= 3` or `% 2`; a bare number means `>= N`. Breakpoints work inside
  `#include`d header files too, not just the top-level source file.
- Object and file browsing (`fluffos/objects`, `fluffos/object`,
  `fluffos/files` custom DAP requests) work while the VM is **running** —
  you don't need to pause the mud to inspect what's loaded.
- Function arguments and locals show their real source names, not `arg0`/
  `local0` placeholders — the compiler captures them automatically whenever
  `debugger port` is set. (A local declared inside a `for`/`switch` block
  that has already exited by the time its function finishes compiling can't
  be recovered this way and falls back to an index name; everything else —
  including object variables — always shows real names.)
- Values can be edited while stopped (DAP `setVariable`): frame arguments
  and locals, object variables, array elements, and mapping values accept a
  new integer, float, or double-quoted string literal. Arrays, mappings,
  objects, and function values aren't assignable this way (that needs a real
  expression evaluator, not yet implemented).

## Connecting from VS Code

See `tools/vscode-lpc-debug/` in the repository for the companion extension.
A minimal `launch.json` attach configuration looks like:

```json
{
  "type": "fluffos",
  "request": "attach",
  "name": "Attach to FluffOS",
  "host": "127.0.0.1",
  "port": 4711,
  "token": "change-me"
}
```

## LPC-facing efuns

- `int debugger_attached()` — returns 1 while a debugger client is attached,
  0 otherwise. Cheap to call from hot paths to guard expensive diagnostics.
- `void debug_break()` — a programmatic breakpoint, similar to JavaScript's
  `debugger;` statement. Pauses the VM at the next instruction when a
  debugger is attached; an exact no-op (safe to leave in shipped code)
  when no debugger is attached.

```c
void process_order(mapping order) {
    if (debugger_attached() && order["total"] < 0) {
        debug_break();  // inspect `order` interactively
    }
    // ...
}
```

## Security

The debugger operates at driver level: it can read and write any object's
variables regardless of mudlib `valid_read`/`valid_write` policy (see
`setVariable` above), and will be able to execute arbitrary code once
expression evaluation ships. Treat `debugger port` like a root shell on the
mudlib:

- Leave it disabled (the default) on production/shared servers.
- Prefer loopback-only binding plus an SSH tunnel over exposing it
  directly, even with a password set.
- Rotate `debugger password` like any other credential.
- Optionally define `master::valid_debugger(remote_ip)` to add mudlib-side
  policy (an IP allowlist, logging, rate-limiting) on top of the password
  check — see [valid_debugger](../../apply/master/valid_debugger). It's
  consulted once per attach and, if undefined, doesn't change behavior at
  all — the password/loopback gates above already apply either way.

## See Also

- [Configuration](../../driver/config.md) — the `debugger port` /
  `debugger address` / `debugger password` options
- [valid_debugger](../../apply/master/valid_debugger) — optional
  mudlib-side veto over attachment
- [Tracing and Performance Profiling](tracing.md) — a complementary,
  non-interactive profiling tool
- [dump_trace](../../efun/internals/dump_trace) — one-shot LPC call stack
  dump, usable without a debugger attached
