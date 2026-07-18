# FluffOS LPC Debugger (VS Code)

Attach VS Code to a running FluffOS driver's WebSocket LPC debugger:
breakpoints, single-stepping, call stack, and variable inspection for LPC.
See `src/debugger/DESIGN.md` for the driver-side architecture.

This extension is a thin bridge — the driver speaks the [Debug Adapter
Protocol](https://microsoft.github.io/debug-adapter-protocol/) directly over
a WebSocket (subprotocol `dap`), so the extension's only job is opening that
connection and mapping mudlib-absolute source paths (`/std/room.lpc`) to and
from files in your workspace. Everything else — the breakpoints UI, call
stack, variables panel — is VS Code's standard debug UI.

Install alongside [`tools/lpc-syntax/vscode`](../lpc-syntax/vscode) for
syntax highlighting; the two extensions are independent.

## Enabling the debugger on the driver

Add to your config file (disabled unless set):

```
debugger port : 4711
```

See the [WebSocket LPC Debugger](../../docs/concepts/general/debugger.md)
guide for the full option reference and security notes — in particular,
prefer loopback binding plus an SSH tunnel for anything but a local dev
server.

## Usage

Add an attach configuration to your workspace's `.vscode/launch.json` (or
use the "FluffOS: Attach to driver debugger" snippet from the Run and Debug
panel):

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "type": "fluffos",
      "request": "attach",
      "name": "Attach to FluffOS",
      "host": "127.0.0.1",
      "port": 4711,
      "token": "",
      "remoteRoot": "/",
      "localRoot": "${workspaceFolder}"
    }
  ]
}
```

| Field | Meaning |
|---|---|
| `host` / `port` | Where the driver's debugger listener is bound. |
| `token` | Shared secret, if the driver sets `debugger password`. |
| `remoteRoot` | The mudlib-absolute root the driver's paths are relative to (usually `/`). |
| `localRoot` | The local folder that mirrors `remoteRoot` — defaults to the workspace root. |

Set a breakpoint in an `.lpc`/`.c` file and start the "Attach to FluffOS"
configuration. Breakpoints set before attaching are sent once the session
initializes; breakpoints on a file the driver hasn't loaded yet stay
"unverified" until it is.

## Non-VS-Code clients

Because the driver speaks plain DAP, any DAP-capable editor works too. For
one that expects the standard stdio transport (`nvim-dap`, for example)
rather than a raw WebSocket, point it at
[`tools/dap-ws-bridge.js`](../dap-ws-bridge.js):

```lua
require('dap').adapters.fluffos = {
  type = 'executable',
  command = 'node',
  args = { '/path/to/fluffos/tools/dap-ws-bridge.js', '--host', '127.0.0.1', '--port', '4711' },
}
```

## Efuns

- `debugger_attached()` — 1 while a client is attached, else 0.
- `debug_break()` — a programmatic breakpoint (like JavaScript's
  `debugger;`); a no-op when nothing is attached.

## Limitations (current phase)

- Local variable and parameter names are not yet available from the driver
  (a compiler limitation, tracked in `src/debugger/DESIGN.md` §9); they show
  as `arg0`, `local1`, etc. Object globals show their real names.
- Expression evaluation (watch, hover, debug console) is not implemented
  yet — phase 3 in the design doc.
- The debugger is a stop-the-world debugger: pausing at a breakpoint pauses
  the entire mud until you continue or disconnect.
