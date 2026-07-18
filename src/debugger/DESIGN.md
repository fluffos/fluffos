# FluffOS WebSocket LPC Debugger — Design & Implementation Plan

Status: **Phases 0-2 IMPLEMENTED.** Listener, breakpoints (incl. pending-until-loaded), stepping,
stack/scope/variable inspection with real local/argument names, `setVariable`, object and file
browsing, exception filters, and the `debug_break()`/`debugger_attached()` efuns all ship today.
Phase 3 (expression evaluation, conditional breakpoints/logpoints, `disassemble`, edit-and-continue,
raw-TCP/wasm transports) remains open. This document is kept as the planning/rationale record —
resolved open questions are struck through in [§17](#17-open-questions--clarifying-questions) rather
than deleted, and phase/status markers are added inline at the sections they affect, but for exact
current behavior the code (and `AGENTS.md` §15, the day-to-day facts) is the source of truth.

---

## 1. Goals and non-goals

### Goals

1. A **debugger endpoint in the driver**: a dedicated WebSocket port speaking a JSON debug
   protocol (proposed: the Debug Adapter Protocol, DAP), disabled by default, auth-gated.
2. **Execution control**: pause / continue, source-line breakpoints (including files not yet
   loaded), step in / over / out, a programmatic `debug_break()` efun, and optional
   break-on-LPC-error.
3. **Stack inspection**: the full LPC control stack (every frame: object, program, file:line,
   function, frame kind), and per-frame arguments, locals, and the owning object's global
   variables, rendered as an expandable value tree.
4. **Object introspection**: enumerate every loaded object (`obj_list`), inspect any object's
   globals, program metadata, clone relationships — independent of where the VM is stopped.
5. **File introspection**: list mudlib source files, fetch source content over the protocol (so a
   remote VS Code without a shared filesystem can still show sources and set breakpoints), and
   enumerate loaded programs (DAP `loadedSources`).
6. **Near-zero overhead when not attached**, and zero behavior change when the feature is disabled
   in the config.
7. A **thin VS Code extension** that bridges the WebSocket to VS Code's debug UI.

### Non-goals (for this effort; some are phased stretch items)

- Debugging the **driver C++ itself** (that's gdb/lldb; `.vscode/launch.json` already covers it).
- **Production-hardened multi-tenant** debugging. This is a development tool: while the VM is
  paused, the whole mud is paused (see §6.1). A read-only "observe" mode is sketched but optional.
- **Time-travel / record-replay**, data watchpoints, multi-client sessions.
- **WASM target support** in the first phases. The design keeps the transport modular so the
  browser build can later reuse the session core over `jsbridge` (§13.6).

---

## 2. Facts about the current driver that constrain the design

Everything below was verified against the tree at the time of writing (file:line references are
anchors, not guarantees against drift).

### 2.1 The VM is single-threaded and pausable between instructions

- The interpreter is `eval_instruction()` in `src/vm/internal/base/interpret.cc` (≈:2045): a
  `while(true)` loop, `instruction = EXTRACT_UCHAR(pc++)`, giant `switch`. All durable VM state is
  file-scope globals: `pc`, `sp`, `fp` (:134-137), `current_prog` (:109), `current_object`
  (simulate.cc:136), `csp`/`control_stack` (:152-154).
- The **loop top is a proven safe observation point**: three hooks already run there each
  instruction — the `DBG_LPC` line tracer (:2078-2084 → `show_lpc_line()`), the `TRACE_CODE` ring
  buffer (:2086-2099), and the Chrome-trace per-instruction tracer (:2113-2115). Mid-instruction is
  *not* safe (opcodes stage `sp` mutations).
- `eval_instruction` is **re-entrant**: nested invocations for external applies
  (`call_program()` macro, interpret.h:88) and for `catch` (`do_catch()` ≈:4494 wraps a recursive
  call in C++ `try`). A debugger must therefore track logical depth via `csp`, never via C++
  stack frames.
- LPC errors are **real C++ throws**: `error()` → `error_handler()` (simulate.cc:2350) →
  `throw (const char*)`, caught in `do_catch` / top-level apply boundaries; VM state is manually
  unwound by `restore_context()` (interpret.cc:5385). Debugger state must survive this unwind.

### 2.2 The eval limit is a wall-clock POSIX timer — it must be suspended while paused

- `max_eval_cost` is **microseconds**, not an instruction count (issue #380 context). `set_eval()`
  (`src/vm/internal/eval_limit.cc:17`) arms a one-shot `timer_create` timer delivering SIGVTALRM
  (`src/vm/internal/posix_timers.cc`); the handler only sets `volatile int outoftime`, checked at
  the loop top (interpret.cc:2101) which then throws "Too long evaluation".
- Consequence: pausing at a breakpoint for minutes is harmless *during* the pause, but the timer
  will have fired and the **first instruction after resume would abort the evaluation**. The pause
  path must save the remaining budget (`get_eval()`), disarm (`posix_eval_timer_set(0)` disarms per
  POSIX `it_value = {0,0}` semantics, and `set_eval` clears `outoftime`), and re-arm on resume.
- SIGVTALRM (and any signal) can **EINTR** a blocking poll — the pause service loop must handle it.
- Non-Linux platforms have *no* eval timer (`init_eval()` prints "Platform doesn't support eval
  limit!") — the disarm/restore must be a no-op there.

### 2.3 One libevent loop drives everything; nothing else dispatches while we hold it

- `backend()` (`src/backend_libevent.cc:110`) runs the **single** `event_base_loop(base, 0)` in the
  tree. Gametick timer (`on_game_tick`, one-shot re-armed), heart_beats
  (`packages/core/heartbeat.cc:35`, self-rescheduling gametick events), call_outs, user command
  timers, DNS, async completions — all are events on that one base.
- Therefore: if the VM pauses inside `eval_instruction` (which sits inside some libevent callback)
  and we service *only* the debugger connection in a private loop, **no heart_beat, call_out, or
  user command can run** — stop-the-world comes for free, with no suppression logic.
- Pleasant side effect to verify at implementation time: gametick-driven schedules effectively
  *freeze* during a pause (`g_current_gametick` only advances in `backend_run_one_gametick`, and
  the one-shot tick timer just pends), so heart_beats/call_outs don't "burst" on resume — at most
  one pending tick fires. Wall-time one-shot events (`event_base_once`) do all fire on resume.
- Cross-thread wake precedent: `add_walltime_event()` → `event_base_once` is thread-safe
  (`evthread_use_pthreads()`, backend_libevent.cc:52) and is how `packages/async` workers wake the
  loop. We don't need threads (§5.3), but the mechanism exists if a threaded transport is ever
  preferred.

### 2.4 The websocket stack: per-port lws contexts bound to the main loop; a standalone context is possible

- Websocket player ports (`PORT_TYPE_WEBSOCKET`, `src/base/internal/external_port.h:15-40`) get an
  lws context each (`init_websocket_context`, `src/net/websocket.cc:55-115`) bound to the **main**
  `g_event_base` via `LWS_SERVER_OPTION_LIBEVENT` + `foreign_loops`, with
  `CONTEXT_PORT_NO_LISTEN_SERVER` — the driver's own `evconnlistener` accepts and
  `lws_adopt_socket_vhost()`s the fd (websocket.cc:117-126). Subprotocols registered: `http`,
  `ascii`, `telnet`, `binary` (websocket.cc:18-25).
- A debugger context can be a **second, independent lws context that omits the event-lib binding**
  and is serviced manually with `lws_service(ctx, timeout_ms)`. lws supports multiple contexts per
  process; the per-context service model is exactly what lets us keep talking to the client while
  the main loop is suspended. Caveats found: `lws_set_log_level` is process-global; the debugger
  context needs its **own** `protocols[]` table (never reuse the shared player one); the driver
  currently never calls `lws_service()` anywhere, so this is a new (but supported) pattern.
- Non-user connections have precedent: HTTP requests on the mount and pre-`ESTABLISHED` ws sockets
  are live lws connections that never become `interactive_t` users. The debugger connection stays
  permanently in that category — it must never enter `all_users` or be visible to LPC.

### 2.5 Line tables are per-program and invertible — source breakpoints are feasible

- `program_t` carries `line_info` (byte-run encoded) + `file_info` (`(lines, file_id)` pairs
  resolving `#include` files), produced by `switch_to_line()` (compiler/internal/icode.cc:281) and
  `save_file_info()` (compiler.cc:3341). Program addresses are **byte offsets into
  `prog->program`**.
- Forward decode exists: `find_line()` (interpret.cc:4769), `get_explicit_line_number_info()`
  (:4817), `get_line_number(pc, prog)` (:4846). The encoding is a walkable list of
  `(run_length, absolute_line)` entries, so the **reverse mapping (file:line → first code address
  of that line) is a straightforward linear walk** — the disassembler already prints exactly that
  table (`dump_line_numbers()`, compiler/internal/disassembler.cc:755).
- Two operational caveats: line info can be **swapped out** (`line_swap_index`) and must be forced
  resident before use; and `#include`d files are addressable via `file_info`/`translate_absolute_line`,
  so breakpoints inside headers are resolvable (phase-2 polish).

### 2.6 Symbols: functions and globals are named at runtime; **locals are not**

- Per program: `function_table` (`function_t`: `funcname`, `num_arg`, `num_local`, `address`),
  `variable_table`/`variable_types` (global names), `inherit[]` with function/variable index
  offsets. Enumerating an object's full global set = `variable_name(prog, i)` +
  `ob->variables[i]` for `i < num_variables_total` (exactly what `debug_info(2, ob)` does,
  packages/develop/develop.cc:103).
- Frame layout: `fp[0..num_arg-1]` = args, `fp[num_arg..num_arg+num_local-1]` = locals
  (`setup_variables`, interpret.cc:1537). For non-top frames, the live registers are read from the
  **next-higher** csp entry (`p[1].fp` trick used by `dump_trace`/`get_svalue_trace`,
  src/vm/internal/trace.cc:96-105).
- **Local/parameter names are discarded after each function body compiles**
  (`free_all_local_names()`, compiler.cc:380-393); only counts survive. So v1 shows `arg0…`,
  `local0…`; real names require a new compiler-emitted table (§9). Class *member* names ARE
  retained (usable for expanding class values).

### 2.7 Existing machinery we will reuse rather than reinvent

| Need | Existing machinery |
|---|---|
| Zero-cost-when-off hook idiom | `Tracer::enabled()` static-flag gating (`src/base/internal/tracing.h:105`, `ScopedTracer`) |
| Per-line hook precedent | `DBG_LPC` block at interpret.cc:2078 |
| Backtrace construction | `get_svalue_trace()` / `dump_trace()` (src/vm/internal/trace.cc:193/:65), `f_call_stack` (packages/core/efuns_main.cc:255) |
| Value preview rendering | `svalue_to_string()` (packages/core/sprintf.cc:319; depth-capped, needs output-length cap) |
| Object enumeration | `obj_list` walk (simulate.cc:132), `ObjectTable::find/children` (otable.cc) |
| Global name→slot | `find_global_variable()` (vm/internal/base/object.cc:1331) |
| Reflection efun templates | `functions()`, `variables()`, `fetch_variable/store_variable` (packages/contrib), `debug_info`, `dump_prog` (packages/develop) |
| Error interception point | top of `error_handler()` (simulate.cc:2350), caught-vs-uncaught decided at :2367 via `FRAME_CATCH` |
| Expression evaluation (phase 3) | lpcshell's `load_object_from_source()` + `compiler_diags` machinery (src/main_lpcshell.cc) |
| Config plumbing | `INT_FLAGS[]`/`STR_FLAGS[]` tables in rc.cc + `runtime_config.h` slots (free: `CFG_STR(19+)`, `CFG_INT(66+)`); regen `docs/driver/config.md` via `docs/gen_config_docs.py` |
| JSON | vendored `nlohmann/json` (src/thirdparty) |
| ws e2e test harness | `tools/ws-smoke.js` (zero-dep node RFC6455 client, already in CI: ci.yml ubuntu clang Debug) |
| Bytecode swap invalidation | `object_t::prog_generation` (object.h:87), bumped by `recompile_object()` (simulate.cc:815, which also *refuses* while the old program is on the control stack) |

### 2.8 Prior art check

No existing debugger/breakpoint/DAP work in the repo or the issue tracker.
`tools/lpc-syntax/vscode/` is a syntax/lint/format extension with no debug adapter — a natural
future host or sibling for the debug extension (§10).

---

## 3. Decision record

Each decision is **Proposed** until reviewed; §17 carries the matching open questions.

| # | Decision | Choice (proposed) | Alternatives considered |
|---|---|---|---|
| D1 | Wire protocol | **DAP (Debug Adapter Protocol) JSON messages, one per WebSocket text frame**, plus namespaced custom requests (`fluffos/*`) for object/file browsing | Custom JSON-RPC with a translating adapter in the extension (more code, no ecosystem); Chrome DevTools Protocol (browser-centric, poor fit) |
| D2 | Transport | **Dedicated debugger port** (new rc option, not one of the 5 `external_port` slots), WebSocket subprotocol `dap`, served by a **standalone, manually-serviced lws context** | Extra subprotocol on player ws ports (couples lifecycles, wrong trust domain, can't be serviced while paused); raw-TCP DAP (VS Code-native, zero extension — cheap to add later behind the same session core, but not what was asked for) |
| D3 | Pause model | **Stop-the-world**, single logical thread. Pause loop services only the debugger socket; eval timer suspended | Threaded transport + condvar handshake (more machinery, no added capability for a single-threaded VM); non-blocking observe-only mode (kept as optional phase-2 sub-mode) |
| D4 | Hook | Always-compiled, **runtime-flag-gated check at the eval loop top** (tracer idiom); tiered fast path (detached → attached-idle → breakpoints → stepping) | `#ifdef` compile-out (loses "attach to a stock binary"; can still be added later); bytecode patching with an `F_BREAKPOINT` opcode (fastest, but invasive; deferred as an optimization) |
| D5 | Locals names | v1 ships **index-named** locals (`arg0`, `local3`); phase 2 (shipped, §9) adds a compiler-emitted local-name table, **always on whenever `debugger port` is set** | Re-parsing source at debug time (fragile); a separate opt-in rc option (one more knob for marginal benefit, since the table only exists at all when a debugger port is already configured) |
| D6 | Expression eval | Deferred to **phase 3**, design sketched (§8.4) | In v1 (pulls in compiler + security surface too early) |
| D7 | Client policy | **Single client**; second connection rejected. Client death ⇒ auto-detach: clear breakpoints, resume, restore timer | Multi-client (no compelling use; complicates stopped-state ownership) |
| D8 | Security default | Disabled unless `debugger port` set; binds `127.0.0.1` by default; optional shared-secret; **refuse non-loopback bind without a secret**; optional master apply veto | Open by default (unacceptable: the debugger is root-equivalent over the mudlib) |
| D9 | Code home | Protocol/session/inspection core in **`src/debugger/`** (like `src/net/`); LPC-visible efuns (`debug_break()`, `debugger_attached()`) in a tiny **`packages/debugger`** spec package | Everything in a package (violates the package≈efuns convention; server needs vm/net internals) |
| D10 | Extension home | **`tools/vscode-lpc-debug/`** in-repo (protocol and client version together), sibling of `tools/lpc-syntax/vscode` | Separate repo; merging into the existing syntax extension (later, once stable) |

---

## 4. Architecture overview

```
                   ┌────────────────────────────── driver process ──────────────────────────────┐
                   │                                                                            │
 VS Code ──ws────► │  evconnlistener (accept only, main event_base)                             │
 extension  :4711  │        │ adopt fd                                                          │
 (DAP client)      │        ▼                                                                   │
                   │  debugger lws context  ◄── serviced by lws_service():                      │
                   │  (standalone, NOT bound     • ~30ms timer on main base   (VM idle)         │
                   │   to main event_base)       • instruction-hook poll      (VM running)      │
                   │        │                    • blocking pause loop        (VM stopped)      │
                   │        ▼                                                                   │
                   │  DebugServer (src/debugger/)                                               │
                   │   ├─ session state machine: DISABLED → LISTENING → ATTACHED ⇄ STOPPED      │
                   │   ├─ DAP dispatch (nlohmann/json)                                         │
                   │   ├─ breakpoint store  ── resolve ──► program_t line tables                │
                   │   ├─ stepping engine   ── reads ────► pc / csp / current_prog globals      │
                   │   └─ inspector         ── reads ────► csp frames, fp slots, obj_list,      │
                   │                                       variable_table, svalue_to_string     │
                   │                                                                            │
                   │  eval_instruction() loop top:  if (UNLIKELY(g_lpc_debugger_flags)) hook()  │
                   │  error_handler() top:          if (attached && exc-filters) maybe_stop()   │
                   │  load_object()/recompile:      notify → resolve pending / rebind bps       │
                   └────────────────────────────────────────────────────────────────────────────┘
```

The **session core is transport-agnostic**: `DebugTransport` is a small interface (send message /
poll for messages / close), mirroring how `Transport` (src/net/transport.h) abstracts user byte
streams. Phase 1 ships one implementation (`transport_lws.cc`); raw-TCP DAP or a WASM/jsbridge
transport can be added without touching the session core.

---

## 5. Transport & session

### 5.1 Configuration (new rc options)

Follows the `INT_FLAGS[]`/`STR_FLAGS[]` table pattern (rc.cc), slots per runtime_config.h free
ranges; regenerate `docs/driver/config.md` with `docs/gen_config_docs.py`; add to
`testsuite/etc/config.test` (commented out except in the debugger e2e job).

| Option | Type/slot | Default | Meaning |
|---|---|---|---|
| `debugger port` | `CFG_INT(66)`, 0–65535 | `0` | 0 = feature fully disabled; otherwise ws listen port |
| `debugger address` | `CFG_STR(19)` | `127.0.0.1` | bind address for the listener |
| `debugger password` | `CFG_STR(20)` | empty | shared secret checked at attach; **required if `debugger address` is non-loopback** (driver refuses to boot the listener otherwise) |

Local/argument name capture (§9) has no separate rc option: it's always on whenever `debugger port`
≠ 0, so there's exactly one knob to remember instead of two.

TLS (`debugger tls : cert=… key=…`, mirroring `external_port_N_tls`) is phase 2; primary use is
loopback/SSH-tunneled.

### 5.2 Listener and connection lifecycle

- Boot: alongside `init_user_conn()`, if `debugger port` ≠ 0, create an `evconnlistener` on the
  main base (accept only) and the standalone debugger lws context (own `protocols[] = {"http",
  "dap"}`, `CONTEXT_PORT_NO_LISTEN_SERVER`, **no** `LWS_SERVER_OPTION_LIBEVENT`).
- Accept → `lws_adopt_socket_vhost` into the debugger context. Plain-HTTP requests get a 404 (no
  mount). Only the `dap` subprotocol upgrades.
- Handshake: DAP `initialize` → capabilities; `attach` carries `{token}` — mismatch or >10s
  handshake timeout ⇒ close with a ws close reason. Second concurrent client ⇒ immediate reject.
- Detach/disconnect (or dead socket, detected via lws ping/pong timeouts): clear all breakpoints,
  cancel stepping, **resume if stopped**, restore the eval timer, return to LISTENING. The mud
  must never stay frozen because a laptop lid closed.
- Driver shutdown while attached: send `terminated`, close cleanly.

### 5.3 Servicing model (the core trick)

One code path, `debugger_service(timeout_ms)` = `lws_service(dbg_ctx, timeout_ms)` + dispatch of
any complete DAP messages, invoked from three places:

1. **VM idle** (driver sitting in `event_base_loop`): a repeating ~30ms timer on the main base
   calls `debugger_service(0)`. Requests are processed between event callbacks — the VM is not
   executing, so all structures are consistent.
2. **VM running** (inside `eval_instruction`): the instruction hook decrements a counter and every
   N instructions (N ≈ 64k, tuned so it costs ~nothing) calls `debugger_service(0)`. This is what
   makes **pause work inside a runaway/infinite LPC loop** — precisely the case a MUD developer
   needs it for, and one the main-base timer alone can never handle (the loop is blocked inside a
   callback).
3. **VM stopped**: the stop loop (§6.2) blocks in `debugger_service(50)` repeatedly (EINTR-safe)
   until a continue/step/disconnect transition.

Requests that require a stopped VM (`stackTrace`, `scopes`, `variables`, `setVariable`, `evaluate`
with a `frameId`) are validated and politely errored when running. Everything else
(`setBreakpoints`, `threads`, `pause`, custom object/file browsing) is legal in any state — note
that object/file introspection therefore works on a **live, running** mud too, satisfying the
"introspect all objects/files" requirement without requiring a pause.

### 5.4 Framing

One complete DAP JSON message per **ws text frame** (no `Content-Length` headers — ws already
frames). The VS Code extension re-frames to VS Code's stream transport. Max inbound frame size
enforced (e.g. 1 MiB); outbound large payloads (variable pages, source content) are bounded by
pagination (§8.2) and lws partial-write handling (evbuffer-drain pattern as in ws_telnet.cc, but
on the private context).

---

## 6. Execution control

### 6.1 VM states and the stop-the-world contract

```
ATTACHED-RUNNING ──(breakpoint | step-complete | pause-arm | debug_break() | error-filter)──► STOPPED
STOPPED ──(continue | step in/over/out | disconnect)──► ATTACHED-RUNNING
```

While STOPPED: the C++ stack is parked inside the instruction hook (inside `eval_instruction`,
inside whatever libevent callback dispatched the LPC). Consequences, by design (§2.3): no
heart_beats, no call_outs, no user commands, no other sockets serviced; player TCP data queues in
kernel/evbuffers; gametick-scheduled work freezes rather than bursting. This is acceptable — and
desirable — for a development tool, and is documented loudly for anyone pointing it at a live game.

Entering STOPPED:
1. Save + disarm the eval timer (`saved_eval = get_eval(); set_eval(0)`), clear `outoftime` (§2.2).
2. Emit DAP `stopped` (reason: `breakpoint` / `step` / `pause` / `exception` / `function
   breakpoint` for `debug_break()`), thread id 1.
3. Enter the blocking service loop.

Leaving STOPPED: drop all variables/frame handles (DAP lifetime rules), re-arm `set_eval(max(saved_eval,
floor))` with a small floor so a nearly-expired budget doesn't instantly abort, emit `continued`,
return from the hook into the dispatch loop.

**Pause while idle**: DAP `pause` arms stop-at-next-instruction. If the VM is idle, nothing stops
until the next event dispatches LPC — the response acks immediately and the `stopped` event fires
on first executed instruction. (An honest limitation; a "freeze the scheduler at idle" mode is a
possible later refinement, but inspecting an empty stack has little value and all *introspection*
already works while running.)

### 6.2 The instruction hook

At the dispatch-loop top, next to the existing `DBG_LPC` block (interpret.cc:2078):

```cpp
// interpret.cc, loop top
if (UNLIKELY(g_lpc_debug_flags)) {           // single u32, 0 when detached
  lpc_debugger_hook();                       // out-of-line, src/debugger/
}
```

`g_lpc_debug_flags` is a bitmask so the fast path stays one load+test+branch when detached — the
same cost class as the three per-instruction checks already present (`debug_level & DBG_LPC`,
`CONFIG_INT(__RC_TRACE_CODE__)`, `CONFIG_INT(__RC_TRACE_INSTR__)`). Inside the hook, tiered work:

| Mode bits | Work per instruction |
|---|---|
| ATTACHED only | `--poll_counter == 0` → `debugger_service(0)` (§5.3) |
| + PAUSE_ARMED | stop unconditionally (one-shot) |
| + BREAKPOINTS | hash-set lookup of `pc-1` (the current opcode address) in the resolved-address set |
| + STEPPING | compute current (file,line) via cached line-run lookup; compare against step baseline (§6.4) |

The hook must **never fire re-entrantly** (a guard flag) and must **do nothing while the error
unwind is in progress** (`num_error > 0` guard) — pausing mid-unwind would observe a half-restored
machine.

Overhead plan: measure attached-idle and breakpoints-set costs with the existing LPC testsuite
timing (`[ RUN ]` per-file times) and `bench_*` harnesses; if the per-instruction hash lookup is
felt in practice, the recorded optimization path is per-program breakpoint presence flags, and
ultimately bytecode patching (replace opcode byte with `F_BREAKPOINT`, original byte in a side
table) — deliberately *not* v1.

### 6.3 Breakpoints

**Identity**: `(canonical mudlib path, line)` — extension-blind path matching, mirroring object
identity rules (`.lpc`/`.c` both match; `filename_to_obname` semantics).

**Resolution** (`setBreakpoints` and on every program load):
1. Canonicalize the requested path.
2. Find candidate `program_t`s: `ObjectTable::find(basename)` for the file's own master object,
   plus a walk of `obj_list` collecting **unique** `ob->prog` and transitive `inherit[]` programs
   whose `prog->filename` (or, phase 2, whose `file_info` include-file strings) match. There is no
   global program registry (§2.5) — the walk is the honest option and is per-`setBreakpoints`,
   not per-instruction.
3. Per program: force line info resident, walk the `line_info` runs accumulating addresses; every
   run whose resolved (file,line) matches gets its start address added. If the line has no code,
   snap to the next line that does and report the adjusted line in the DAP `breakpoint` response
   (`verified: true, line: actual`); if the file isn't loaded anywhere, keep the breakpoint
   **pending** (`verified: false`).
4. Maintain: `unordered_set<char*>` of live addresses (the hook's fast lookup) + a per-program
   registry for invalidation.

**Invalidation & rebinding**:
- Program freed → remove its addresses (hook `deallocate_program`/`free_prog`).
- `recompile_object()` swap → old program's entries die with it; re-resolve against the new
  program (bumped `prog_generation` is the signal; note the driver already refuses to swap a
  program that is on the control stack, so a *stopped-inside* program can't be swapped out from
  under the debugger — simulate.cc:838-850).
- New program compiled (`load_object` success path) → try pending breakpoints; on success emit DAP
  `breakpoint` event (`verified: true`).

**`debug_break()` efun** (packages/debugger): no-op when detached; when attached, sets PAUSE_ARMED
so the *next* instruction stops (i.e. it behaves like a `debugger;` statement). Also
`debugger_attached()` → 0/1 so mudlibs can guard expensive diagnostics.

**Conditional / hit-count breakpoints**: hit-count conditions are phase 2 (pure counter);
expression conditions depend on the evaluator, phase 3.

### 6.4 Stepping

Baseline captured at the step command: `base_csp` (logical depth — valid because `csp` is one
global array across nested `eval_instruction`s), and current `(file, line)`.

| Command | Stop when |
|---|---|
| `stepIn` | (file,line) changed, or a frame was pushed |
| `next` (over) | (file,line) changed **and** `csp <= base_csp` |
| `stepOut` | `csp < base_csp` |

Details:
- Line lookups only happen in STEPPING mode, with a cached current-run `[addr_lo, addr_hi) → line`
  so the common same-line case is two pointer compares.
- **Unwind during a step** (an `error()` popped frames past the baseline): treat as the stop
  condition (you land at the catch or at the next entry), and reset cleanly if the whole
  evaluation unwinds.
- **Stack runs dry** (the stepped frame returned out of the outermost LPC entry): stepping arms
  *stop-at-next-LPC-entry*, so `next` at the end of a heart_beat lands at the top of the next
  dispatched LPC event (with the `stopped` event carrying context). This "step across the event
  boundary" is unusually useful in an event-driven mud; it will be the default, with a client
  setting to make it `continue` instead.
- DAP `stepInTargets`/instruction-granularity stepping: out of scope v1 (a `disassemble`-view mode
  reusing `dump_prog`'s disassembler is a tempting phase-3 stretch).

### 6.5 Break on LPC error

Hook at the top of `error_handler()` (simulate.cc:2350), **before** the `FRAME_CATCH` decision
(:2367), where the full control stack is still intact (same reason `mudlib_error_handler` can
collect `get_svalue_trace()` there). DAP `setExceptionBreakpoints` filters:

| Filter | Meaning |
|---|---|
| `uncaught` (default on) | stop only when no `FRAME_CATCH` protects the throw |
| `all` | stop on every `error()` / `throw()`, including ones a `catch()` would absorb |

On stop: `stopped(reason: exception)` + `exceptionInfo` carrying the error text and the would-be
catch/uncaught disposition. Resume continues the normal unwind. Guards: never re-enter while
`num_error > 0` (error-inside-error), and `too_deep_error`/`max_eval_error` stops must first
stabilize the respective limit (re-arm eval timer before the stop loop) — same handling the
mudlib-error-handler path already needs.

---

## 7. Threads model (DAP)

One thread, id 1, name `"LPC"`. All `stopped`/`continued` events reference it. (call_out queues,
heart_beat sets etc. are *data*, exposed via custom requests later if wanted — not threads.)

## 8. Inspection

### 8.1 Stack frames (`stackTrace`)

Walk `csp` down to `control_stack[0]` exactly like `get_svalue_trace()` (including the
top-frame-uses-globals / `p[1]` register trick, trace.cc:96-105). Per frame:

- `name`: function name (`function_table[fr.table_index].funcname`; `FRAME_FUNP` → `<function>`,
  `FRAME_CATCH` → `catch`, `FRAME_FAKE` labeled), plus the object name as presentation hint
  (`/std/monster#42 → attack()`).
- `source`/`line`: topmost frame from `current_prog`+`pc`; lower frames from the saved return `pc`
  (as `f_call_stack` mode 4 does).
- `presentationHint`: `subtle` for `FRAME_CATCH`/`FRAME_FAKE` frames.
- Paged via DAP `startFrame`/`levels` (`supportsDelayedStackTraceLoading`).

### 8.2 Scopes and variables

Per frame, three scopes:

| Scope | Source | Naming |
|---|---|---|
| Arguments | `frame_fp[0 .. num_arg)` | phase 1 `arg0…`; phase 2 real names (§9) |
| Locals | `frame_fp[num_arg .. num_arg+num_local)` | phase 1 `local0…`; phase 2 real names |
| Object globals | `frame_ob->variables[i]`, names via `variable_name(prog, i)` | always real names (includes inherited) |

Value tree:
- `variablesReference` handle table is **per-stop, cleared on resume** (matches DAP lifetime).
  Handles record *paths* (frame/slot or container pointer + key) — safe while stopped because
  nothing runs; after any state-mutating request (`setVariable`, phase-3 `evaluate`) an
  `invalidated` event is sent.
- Compound expansion: arrays (indexed children, paged with `start`/`count`,
  `supportsVariablePaging`), mappings (key preview → value child), classes (member names are
  available in the program), objects (name + expandable globals), functions (`funptr` kind +
  owner), buffers (hex preview page).
- Previews via `svalue_to_string` into a **capped** outbuffer (it has a recursion cap but no
  length cap — the debugger supplies e.g. 256-char truncation with `… (+N more)`).
- Cycles are safe by construction (children are enumerated lazily one level at a time).
- The inspector holds **no LPC references across resume** — everything is borrowed while stopped.
  This deliberately keeps the debugger invisible to the Debug-build ref-count checker (AGENTS §3):
  no `*_mark` machinery needed until phase 3's evaluate results (which are freed before resume).

`setVariable` (phase 2 -- IMPLEMENTED): re-walks the same container cases `variables` renders to
resolve the target `svalue_t*` by the exact `name` a prior `variables` response gave it (frame
args/locals by name -- real if captured, §9, else `argN`/`localN` -- object globals by name, array
and class elements by their `"[i]"`/`"member [i]"` display text, mapping VALUES by their KEY's
(possibly-truncated) preview text). The mapping case is therefore best-effort: a preview collision
(two keys rendering identical or truncated-identical text) is rejected as ambiguous rather than
risking a write to the wrong entry. The new value is parsed by a deliberately tiny literal grammar
-- integer, float, or a double-quoted string (with `\" \\ \n \t \r` escapes) -- covering "rejects
writes that would need compilation" by construction: arrays/mappings/objects/function literals
have no textual form here and always fail cleanly (phase 3's `evaluate` is the real answer for
those). Assignment goes through `assign_svalue()` for correct ref handling, exactly as sketched.

### 8.3 Objects & files (custom requests + standard bits)

Standard DAP:
- `loadedSources`: unique `prog->filename` over an `obj_list` walk (one entry per program, master
  path normalized).
- `source`: return file content by mudlib path (driver-level read, same file layer as
  `read_file`) so a remote client can display sources and set breakpoints without a shared
  filesystem. Optional `checksums` for client-side staleness display.

Custom (`fluffos/*`, surfaced in VS Code via `customRequest` + a TreeView):
- `fluffos/objects` `{filter?, cursor?, limit}` → page of `{name, program, flags:
  clone|destructed-pending, gen}` from `obj_list` — **works while running** (§5.3).
- `fluffos/object` `{name}` → globals (name/type/preview, expandable one level per call),
  program info (inherits, function count, sizes — `debug_info(1)`-style), clone family
  (`ObjectTable::children`). Re-looked-up by name per request, so destruction between requests is
  a clean error, never a dangling pointer.
- `fluffos/files` `{dir}` → directory listing (driver-level, mudlib-rooted, `..`-escape-proof).
- `fluffos/eval` — phase 3 alias of `evaluate` for non-frame (REPL) contexts.

Note the trust model: these bypass mudlib `valid_read`/`valid_write` — the debugger operates at
driver level. That is exactly why §11 exists.

### 8.4 Expression evaluation (phase 3 sketch)

Reuse lpcshell's approach (`load_object_from_source()`; diagnostics via `compiler_diags` with
`compiler_diags_quiet`): compile a throwaway object containing
`mixed __dbg_eval(<typed params named after the frame's args+locals>) { return (<expr>); }`,
call it with the frame's slot values (reference types arrive shared, so `arr[3] = x` style edits
naturally propagate; scalar copy-back on request), in the context/privilege of the stopped object
(globals reachable via `fetch_variable`/`store_variable`-style intrinsics or generated accessors).
Runs inside a `catch`-equivalent guard with its own small eval budget; any `stopped` recursion is
suppressed while evaluating. Depends on the local-name table (§9) for parameter naming. Gated by
its own config/attach capability because it is arbitrary code execution by design.

---

## 9. Compiler addition: local variable name tables (phase 2 -- IMPLEMENTED)

Problem: names die at `free_all_local_names()` (compiler.cc:380). Shipped design (simpler than the
original sketch below the line):

- `rule_func()` (`compiler/internal/grammar_rules.cc`), right at the top of its `fun != -1` branch
  (before any early-return in the default-argument-closure loop can skip it, and well before
  `free_all_local_names()` fires later in the same function), snapshots `locals_ptr[]` into a plain
  `short* function_t::local_names` array sized `num_arg + num_local`, indexed by
  `locals_ptr[i].runtime_index` (NOT array position -- see "Scoping honesty" below), one
  `store_prog_string()`-interned string-pool index per slot, `-1` for an uncaptured slot. Gated on
  `lpc_debugger_wants_local_names()` (`debug_hook.h`) alone, which is `debugger port ≠ 0` -- no
  separate rc option (open question 8 resolved: always-on, not opt-in).
- **Not a new `A_LOCAL_NAMES` program section.** Unlike line numbers/classes/etc., functions get
  *sorted* between compile-time and final program order (`func_index_map[]` in `epilog()`), so a
  section needing that same reordering is more naturally carried as a field directly on
  `function_t` -- it rides through `prog->function_table[i] = *FUNC(func_index_map[i]);`'s
  struct-copy for free, exactly like `funcname` already does. The array itself is its own
  heap block (`TAG_LOCAL_NAMES`, `src/base/internal/debugmalloc.h`), freed in
  `deallocate_program()` and in `epilog()`'s superseded-entry cleanup, marked in
  `checkmemory.cc`'s `TAG_PROGRAM` case (Debug builds) so it doesn't show up as an orphan block.
- **Scoping honesty**: a local declared inside a `for()`/`switch()` block calls `pop_n_locals()`
  when that block closes, which drops it from `locals_ptr[]` (unrecoverable by the time
  `rule_func()` captures) *without* giving its runtime slot back (`max_num_locals` only grows) --
  so the table can have real gaps (`-1`) in the middle for slots later locals never reuse. No
  pc-range refinement was needed to make this tolerable in practice; the debugger just falls back
  to `argN`/`localN` for a gapped slot. `src/tests/test_debugger.cc`'s
  `LocalNamesLeaveGapForScopedOutLoopVariable` pins this shape.
- Programs compiled with `debugger port` unset simply show index names — the debugger degrades
  gracefully per-program. `functions()`-style reflection is untouched. Anonymous/lambda functions
  (`rule_primary_expr_anon_func()`) are not instrumented -- `rule_func()` is the only capture
  site -- so funptr frames always show index names, a known v1 gap.

---

## 10. VS Code extension (`tools/vscode-lpc-debug/`)

Thin by design — the driver speaks DAP, so the extension is mostly plumbing:

- `package.json` contributes debugger type `fluffos` with an **attach** configuration:
  `{host, port, token, remoteRoot (mudlib path, default "/"), localRoot (workspace folder),
  stopOnEntry?: false}` and breakpoint enablement for LPC files (`.c`, `.lpc`, `.h`).
- `DebugAdapterDescriptorFactory` → `DebugAdapterInlineImplementation` that opens the WebSocket
  (subprotocol `dap`), forwards DAP messages 1:1, and performs **path mapping** both ways
  (mudlib-absolute `/std/room.c` ⇄ `${localRoot}/std/room.c`); falls back to the DAP `source`
  request (sourceReference) when a file has no local counterpart.
- An **"LPC Objects" TreeView** backed by `fluffos/objects` / `fluffos/object`, with a "pin
  object" → live-refresh affordance, and a command palette entry for `fluffos/files` browsing.
- Works alongside (not inside) `tools/lpc-syntax/vscode`; merging later is a packaging decision.
- Because the driver speaks standard DAP, **nvim-dap / other DAP clients work too** with a
  ~10-line ws↔stdio shim (worth shipping as `tools/dap-ws-bridge.js` — it doubles as the e2e
  test's transport and gives non-VS-Code users a path on day one).

## 11. Security model

The debugger is **root on the mudlib**: it reads any object's variables (passwords included),
phase 2 writes them, phase 3 executes arbitrary code. Therefore:

1. Feature is dead code unless `debugger port` is configured (default 0). No LPC or network
   surface exists when disabled.
2. Default bind `127.0.0.1`. Non-loopback bind **refuses to start** without `debugger password`.
   TLS optional in phase 2 for non-tunneled remote use.
3. Shared-secret check inside the DAP `attach` request; failures close the socket (rate-limited
   logging, no oracle).
4. Optional master apply veto (phase 2): `valid_debugger(remote_ip)` consulted at attach —
   mudlibs can layer policy without driver rebuilds.
5. Single client (D7); attach/detach events logged via `debug_message` and a `debug_level`
   channel (`DBG_debugger` bit) for auditability.
6. The debugger deliberately bypasses `valid_read`/`valid_write` (§8.3) — documented, not hidden.

## 12. Performance budget

| State | Cost target | Mechanism |
|---|---|---|
| Disabled (port 0) | zero: no listener, no context, flag stays 0 | one dead branch per instruction (same class as existing DBG_LPC/TRACE checks) |
| Attached, idle mud | ≈0: 30ms timer + `lws_service(0)` | §5.3(1) |
| Attached, running, no bps | one branch + counter decrement per instruction; periodic `lws_service(0)` every ~64k instructions | §5.3(2) |
| Breakpoints set | + one hash-set probe per instruction | §6.2 tiering; optimize later if measured |
| Stepping | + cached line-range compare per instruction | only while a step is in flight |

Validation: run the LPC testsuite (its per-file `[ RUN ]` timings) and `src/tests/bench_*` in all
four states on RelWithDebInfo; record numbers in the PR. Any regression in the *disabled* state is
a blocker by definition.

## 13. Failure modes & edge cases (checklist for implementation/tests)

1. **Client dies while STOPPED** → lws ping timeout → auto-detach: breakpoints cleared, timer
   restored, VM resumed (§5.2). The mud must never require a client to un-freeze.
2. **Eval timer** — fired-during-pause must not abort on resume (§2.2/§6.1); `set_eval_limit` efun
   changing the budget mid-session; platforms without timers.
3. **Error unwind** across a step baseline; error-inside-error; stops requested during unwind are
   suppressed (§6.2).
4. **`recompile_object` / `replace_program`** — stale address invalidation via program free +
   `prog_generation`; re-resolve; driver already refuses swapping on-stack programs.
5. **Object destructed** between two `fluffos/object` requests (name re-lookup; clean error), or
   while its globals scope is open in a stopped session (frames pin nothing: scope reads re-check
   `O_DESTRUCTED` via the csp-held object which cannot be freed while on-stack — verify with a
   test).
6. **Line info swapped out** (`line_swap_index`) at resolution time → force-load.
7. **`#include` lines** — phase 1 verifies/snap-adjusts only same-file lines; phase 2 resolves
   header breakpoints via `file_info`.
8. **Deep/hostile data** in variable rendering — depth caps, pagination, length caps
   (AGENTS §13 recursion rules apply to the inspector too).
9. **Signals**: SIGVTALRM EINTR in the pause loop; shutdown signals during STOPPED (proceed with
   shutdown: send `terminated`, restore, exit).
10. **testsuite/Debug builds**: `check_memory()` must stay clean — guaranteed by the
    no-refs-held-across-resume rule (§8.2); an attached-debugger testsuite run on Debug is part of
    the e2e matrix.
11. **WASM**: `src/debugger` is excluded from the wasm link (stub singleton, matching the
    per-target link-time selection convention — no `#ifdef __EMSCRIPTEN__` in shared logic);
    revisit with a jsbridge transport later.

## 14. Testing & CI plan

- **GTest (`src/tests/test_debugger.cc`)**: line-table inversion round-trip (compile fixtures via
  the existing test_compiler harness; assert file:line→addr→file:line identity incl. snap-to-next
  and include-file cases); breakpoint store invalidation on program free; stepping state machine
  as pure logic (feed synthetic (csp,line) traces).
- **Node e2e (`tools/dap-smoke.js`)**, modeled byte-for-byte on `tools/ws-smoke.js` (boot
  `build/src/driver testsuite/etc/config.test` with `debugger port`, parse ports from config, wait
  for "Initializations complete"): attach handshake (+auth-reject negative test), setBreakpoints
  on a testsuite file, trigger execution via the telnet port, assert `stopped(breakpoint)`,
  stackTrace/scopes/variables sanity, step in/over/out transcript, pause during a deliberate
  spin-loop (the runaway-loop rescue), error-filter stop, disconnect-resumes-world (heart_beats
  observed after detach). CI: same job slot as ws-smoke (ubuntu clang Debug), plus one
  RelWithDebInfo run.
- **LPC testsuite** additions (`testsuite/single/tests/efuns/debug_break.lpc`): `debug_break()` and
  `debugger_attached()` are exact no-ops when detached (the suite never attaches).
- **Manual matrix** per AGENTS §13: Debug+ASan/UBSan and RelWithDebInfo, suite run 2–3×; the
  soak case is attach→run full testsuite→detach with breakpoints set but unhit.

## 15. Phasing

| Phase | Contents | Exit criterion |
|---|---|---|
| **0 — Foundation** | rc options, listener + standalone lws context + `dap` subprotocol, DAP handshake/capabilities, session state machine, instruction hook + flags word, pause/continue, eval-timer suspend/restore, `debug_break()` + `debugger_attached()` efuns, auto-detach safety | VS Code attaches, `debug_break()` stops the mud, continue resumes it, killing the client un-freezes the mud |
| **1 — Core debugging (MVP)** | breakpoint store + resolution + pending + rebinding, stepping engine, stackTrace/scopes/variables (index-named locals, named globals), `source`/`loadedSources`, dap-smoke e2e in CI | set a breakpoint in VS Code, hit it from a player command, step through a heart_beat, inspect values |
| **2 — Quality** | ~~compiler local-name tables~~, ~~`setVariable`~~, ~~exception filters (uncaught/all)~~, ~~object explorer + file browsing custom requests~~ (all IMPLEMENTED) -- remaining: TreeView (extension-side UI), hit-count conditions, master `valid_debugger` veto, TLS option, header-file breakpoints | daily-driver debugging UX |
| **3 — Advanced** | `evaluate`/watch/hover/REPL (lpcshell-derived), conditional breakpoints, logpoints, `disassemble` view (dump_prog), edit-and-continue flow with the hot-reload daemon (breakpoints re-verify after `recompile_object`), raw-TCP DAP listener, wasm/jsbridge transport exploration | power-user parity |

Rough shape: each phase is a reviewable PR series; phase 0+1 are the meaningful MVP.

## 16. File-by-file change inventory (phases 0–1)

| Path | Change |
|---|---|
| `src/debugger/` (new) | `debug_server.{h,cc}` (session, DAP dispatch), `transport_lws.{h,cc}` (standalone context, framing), `breakpoints.{h,cc}` (resolution/inversion), `stepping.{h,cc}`, `inspect.{h,cc}` (frames/scopes/variables/objects/files), `hook.h` (inline fast path + `g_lpc_debug_flags`) |
| `src/vm/internal/base/interpret.cc` | ~6 lines: hook at loop top; expose a tiny frame-walk helper if `trace.cc` internals need factoring |
| `src/vm/internal/simulate.cc` | `error_handler()` intercept; program-load/`recompile_object` notifications |
| `src/vm/internal/base/program.cc` (or allocator site) | program-free notification for breakpoint invalidation |
| `src/vm/internal/eval_limit.cc` | `debugger_suspend_eval()` / `debugger_resume_eval()` (save/disarm/restore) |
| `src/net/transport_libevent.cc` (or `mainlib.cc` boot path) | create debugger listener alongside `init_user_conn()` |
| `src/base/internal/rc.cc`, `src/include/runtime_config.h` | new options (§5.1); `docs/driver/config.md` regen |
| `src/base/internal/log.h/.cc` | `DBG_debugger` level bit |
| `src/packages/debugger/` (new) | `debugger.spec` (`debug_break`, `debugger_attached`), thin `debugger.cc`, `CMakeLists.txt` |
| `src/CMakeLists.txt` | add `src/debugger/` sources (native targets only; wasm gets the stub) |
| `src/tests/test_debugger.cc` (new) | unit tests (§14) |
| `tools/dap-smoke.js`, `tools/dap-ws-bridge.js` (new) | e2e + generic DAP-client shim |
| `tools/vscode-lpc-debug/` (new) | extension (§10) |
| `.github/workflows/ci.yml` | dap-smoke step next to ws-smoke |
| `testsuite/single/tests/efuns/debug_break.lpc` (new) | detached no-op pin |
| `docs/` | concepts page `docs/concepts/general/debugger.md` + sidebar regen (docs phase, end of phase 1) |

## 17. Open questions / clarifying questions

Highest-leverage first; each with the recommendation the plan currently assumes.

1. **Protocol commitment (D1)** — Is DAP-over-WebSocket the right call, or do you want a bespoke
   JSON protocol (with the VS Code extension translating)? DAP buys the whole client ecosystem
   (VS Code, nvim-dap, Theia) and a stable spec, at the cost of conforming to its
   request/response shapes and pushing mud-specific features into custom `fluffos/*` requests.
   *Recommendation: DAP.* Related: is a raw-TCP DAP listener (zero-extension attach via
   `"debugServer"` in launch.json) worth shipping alongside ws, or ws-only?
2. **Deployment intent & pause semantics (D3)** — Is stop-the-world acceptable as the only pause
   model (dev servers), or do you also need a production-safe **inspect-only mode** (object/file
   browsing + stacks-on-error snapshots, but stepping/pause disabled by config)? The design
   supports the read-only subset while running either way; the question is whether "no pause"
   needs to be an enforced server-side mode.
3. **MVP cut (D5/D6)** — Confirm phase 0+1 as the MVP: breakpoints/stepping/stack/variables with
   *index-named* locals (`local0`), named globals — with real local names (compiler work) and the
   object-explorer UI in phase 2, and `evaluate`/REPL in phase 3. Or must local names / evaluate
   be in the first shippable cut?
4. **Extension packaging (D10)** — In-repo `tools/vscode-lpc-debug/` (recommended), a separate
   repository, or contribute debugging into the existing `tools/lpc-syntax/vscode` extension from
   the start?
5. **Auth strength** — Is shared-secret + loopback-default sufficient for v1, with TLS and the
   master-apply veto in phase 2? (Anything stronger — e.g. per-user tokens — seems out of scope.)
6. **Single-client policy (D7)** — reject the second client, or should a new attach take over
   (kicking the old one)? Recommendation: reject; takeover is a two-line change later.
7. **Step-past-event-boundary default (§6.4)** — when a step reaches the end of the outermost
   frame, stop at the next LPC entry (recommended) or continue free?
8. ~~**`debugger local names` cost` (§9)**~~ — **Resolved**: always-on whenever `debugger port` is
   set, no separate rc flag. One knob instead of two; the memory cost (one `short` per local slot,
   only for programs compiled while a debugger port is configured) wasn't worth a second option.
9. **Break-on-error default** — ship with `uncaught` filter pre-enabled on attach (recommended) or
   fully opt-in per session?
10. **Port number convention** — any preferred default suggestion for docs/examples (the plan uses
    4711 in examples; the option itself defaults to disabled)?

## Appendix A — example session (phase 1 flow)

```
client → initialize                      ← capabilities (lines, exceptionFilters, loadedSources…)
client → attach {token}                  ← attach ok; event: initialized
client → setBreakpoints /cmds/wiz/bug.c:42   ← [{verified:true, line:42}]
client → setExceptionBreakpoints [uncaught]  ← ok
client → configurationDone               ← ok                     (mud keeps running)
  … player types "bug" …
                                          ← event: stopped {reason: breakpoint, threadId: 1}
client → stackTrace                      ← frames: cmd_bug@/cmds/wiz/bug.c:42, …, catch, heart_beat
client → scopes {frame 0}                ← Arguments / Locals / Object globals
client → variables {Locals}              ← local0: mapping[12] {…preview…}, local1: 42, …
client → next                            ← event: continued … event: stopped {reason: step}
client → continue                        ← event: continued      (world resumes, timers re-armed)
client → disconnect                      ← breakpoints cleared, detached, mud unaffected
```

## Appendix B — why not just gdb / existing efuns?

`call_stack()`, `dump_trace()`, `debug_info()`, `trace()` and the Chrome-trace profiler are
post-hoc or in-band: they run *as LPC/driver code from inside the mud* and cannot stop time,
step, or inspect an arbitrary paused frame interactively. gdb on the driver sees C++ frames, not
LPC ones. The debugger endpoint composes the two: driver-level access with source-level LPC
semantics, over a protocol editors already speak — and every piece of it reuses the introspection
machinery those existing tools already proved out (§2.7).
