# /std/tui — the LPC TUI library

A terminal UI toolkit written in pure LPC, layered like `readline` +
`ncurses` + `pterm`: line editing / history / incremental search for the
prompt line, inline select/multiselect/confirm prompts and pretty-printers
for ordinary output, plus a screen buffer, diff renderer, and widget set for
full-screen TUIs. Lives in `/std/tui/` of the testsuite mudlib and is
portable to any FluffOS mudlib.

**Try it**: boot the testsuite (`driver etc/config.test`), telnet to port
4000, and run the showcases:

| Command | Shows |
|---|---|
| `tuidemo` | readline prompt: editing, ↑/↓ history, Ctrl-R search, Tab completion |
| `tuidemo select` | inline select → multiselect → confirm prompt chain |
| `tuidemo print` | the pterm-style printers, in plain line mode |
| `tuidemo app` | minimal full-screen app (list + textfield) |
| `tuidemo dashboard` | animated spinner, progress bars, sparkline, live table + log |
| `tuidemo form` | textfield, radio group, checkbox list, buttons |

## 1. Driver substrate (research summary)

Everything the library needs already exists in the driver, with the quirks
fixed as part of this work (see §6):

| Capability | Mechanism |
|---|---|
| Raw keystrokes | `get_char(cb, flags)` — one key per callback, must be re-armed from inside the callback to stay in char mode (`src/comm.cc` `call_function_interactive` reverts to line mode otherwise). Flag `I_NOECHO` (0x1) keeps client echo off across re-arms. |
| Echo control | `I_NOECHO` → driver sends telnet `IAC WILL ECHO` (`set_localecho`). |
| Terminal size | `request_term_size()` → client NAWS → apply `window_size(int width, int height)` on the interactive object; re-fires on every resize. No query efun — the library caches it. |
| Terminal type | `start_request_term_type()` → apply `terminal_type(string term)`. |
| Raw ANSI output | `write()`/`tell_object()`/`receive()` pass ESC (0x1b) through unmolested; telnet layer only rewrites `\n`→CRLF (desirable) and doubles IAC (0xFF). |
| Display width | `strwidth(s)` (UAX#11 columns), width-aware `sprintf` padding. Neither skips ANSI escapes — the library provides `visible_width()`. |
| Timers | `call_out_walltime(fp, float)` for the lone-ESC disambiguation timeout. |

Constraints that shaped the design:

* **One byte-ish event per callback, one pending handler per user.** Escape
  sequences (`ESC [ A` = arrow-up) arrive as separate `get_char` callbacks, so
  the key decoder is an explicit state machine, and a *real* lone ESC can only
  be distinguished by a short timeout (industry standard; we use a walltime
  call_out driven by the glue layer, keeping the decoder itself time-agnostic
  and unit-testable).
* **No prompt interference:** while a `get_char` is pending the driver never
  prints prompts, so the library owns the prompt line entirely.
* **Char/echo/NAWS negotiation requires a telnet-speaking transport** (raw
  telnet, websocket `telnet` subprotocol, or the WASM bridge — all supported;
  the websocket `ascii` subprotocol is not).
* The driver must not be configured with `__RC_NO_ANSI__` +
  `__RC_STRIP_BEFORE_PROCESS_INPUT__` (that rewrites inbound ESC to a space).

## 2. Architecture

Five modules, strictly layered; each lower layer knows nothing about the ones
above. Everything except `terminal.lpc` is a **pure state machine** — no
efuns with side effects, no interactivity — so the whole stack is exercised by
the non-interactive testsuite.

```
             ┌──────────────────────────────────────────────────────┐
   apps      │  /command/tuidemo.lpc  + /clone/tuidemo_*.lpc demos  │
             ├──────────────────────────────────────────────────────┤
   glue      │  terminal.lpc   (get_char loop, NAWS/TTYPE, ESC      │
             │    timeout, cleanup; inherit into user objects;      │
             │    tui_readline / tui_select / tui_confirm / tui_open)│
             ├──────────────┬──────────┬────────────────────────────┤
   engines   │ readline.lpc │ menu.lpc │ screen.lpc  widget.lpc     │
             │ (line editor)│ (select) │ (+ w/* widget set)  app.lpc│
             ├──────────────┴──────────┴────────────────────────────┤
   input     │  keys.lpc   (bytes → key events)                     │
             ├──────────────────────────────────────────────────────┤
   output    │  ansi.lpc (escapes, widths)   print.lpc (printers)   │
             └──────────────────────────────────────────────────────┘
              include/tui.h  (key codes, states, shared macros)
```

### 2.1 `ansi.lpc` — escape sequences & width (stateless, inheritable)

Builders for the sequences a TUI needs: cursor addressing/motion (`CUP`,
`CUU/CUD/CUF/CUB`), erase (`ED`, `EL`), SGR color/attribute (16-color,
256-color, truecolor), alternate screen (`?1049`), cursor show/hide, SGR mouse
reporting on/off, bracketed-paste on/off. Plus the width toolkit the driver
lacks: `visible_width(s)` (display columns ignoring CSI/OSC runs),
`visible_pad(s, w)`, `strip_ansi(s)`, and `width_slice(s, start_col, ncols)`
(codepoint slice by display column, wide-char aware) used by readline's
horizontal scrolling and screen clipping.

### 2.2 `keys.lpc` — key decoder (clonable state machine)

`feed(string chunk)` consumes what `get_char` delivered and returns an array
of decoded events; `flush()` resolves a dangling ESC (called by the glue's
timeout); `has_pending()` tells the glue whether to schedule that timeout.

Event representation (no classes — portable across programs):

* **string** — printable text (one character; a whole paste in one event).
* **int** — everything else. Special keys are `0x110000 + n` (above the
  Unicode range so they never collide with a codepoint); modifiers are bits
  above that: `TUI_MOD_SHIFT/ALT/CTRL`. `Ctrl-A` is `TUI_MOD_CTRL | 'a'`,
  `Alt-b` is `TUI_MOD_ALT | 'b'`, Ctrl-Right is `TUI_MOD_CTRL | TUI_KEY_RIGHT`.
* **array** — mouse: `({ TUI_EV_MOUSE, button, x, y, pressed })` from SGR
  (1006) reports.

Grammar handled: CSI sequences with numeric params and modifier encoding
(`ESC [ 1 ; 5 C`), SS3 (`ESC O P` F-keys/keypad), `ESC [ n ~` legacy keys,
Alt-prefixed printables, bracketed paste (`ESC [ 200~ … 201~` → single text
event), SGR mouse (`ESC [ < b;x;y M/m`), and bare control bytes (CR→ENTER,
TAB, BS/DEL→BACKSPACE, C-a…C-z). Unknown sequences are swallowed and reported
as `TUI_KEY_UNKNOWN` rather than leaking bytes into the app.

### 2.3 `readline.lpc` — line editor (clonable state machine)

The prompt-line centerpiece. Holds buffer, cursor (codepoint index), history,
kill buffer, search state, and a horizontal-scroll viewport; consumes key
events, accumulates ANSI output to repaint **its own line only** (no absolute
cursor addressing — works at any scroll position using `\r`, `EL`, `CUF`).

API (engine only — the glue below wires it to a real user):

```c
void   rl_set_width(int cols);            // from NAWS; re-render on resize
void   rl_set_prompt(string p);           // may contain ANSI colour
void   rl_set_history(string *lines);     // oldest first; shared/persistable
void   rl_set_completer(function f);      // f(text, point) -> string *cands
void   rl_set_masked(int on);             // password mode: render '*'s
string rl_begin(string initial);          // reset + initial paint
int    rl_feed(mixed ev);                 // TUI_RL_{MORE,DONE,ABORT,EOF}
string rl_take_output();                  // drain pending repaint bytes
string rl_line();                         // the accepted / current line
string *rl_history();
```

Default keymap (emacs/readline):

| Keys | Action |
|---|---|
| printables / paste | insert at cursor |
| ←/→, C-b/C-f | move by char; C-←/→, M-b/M-f by word |
| Home/End, C-a/C-e | line start/end |
| BS / Del, C-d | delete back / forward (C-d on empty line = EOF) |
| C-w / M-d / C-k / C-u | kill word-back / word-fwd / to-end / whole line, into kill buffer |
| C-y | yank kill buffer |
| C-t | transpose chars |
| ↑/↓, C-p/C-n | history prev/next (current edit is preserved at the bottom) |
| C-r / C-s | incremental history search (reverse/forward); type to refine, repeat to step, Enter accepts, C-g restores, other keys accept + execute normally |
| Tab | completion: insert longest common prefix; on ambiguity, list candidates above the line |
| Enter | accept → `TUI_RL_DONE` |
| C-c | abort → `TUI_RL_ABORT` |
| C-l | clear screen and repaint the line |

Rendering: single logical line with horizontal scrolling when
`visible_width(prompt+buffer) >= cols` (busybox/readline `--horizontal-scroll`
style, with `<`/`>` scroll markers). Wide (CJK) characters are handled via
column-aware slicing. Repaint is a full line rewrite (`\r` + prompt + view +
`EL(0)` + cursor reposition) — simple, correct, and cheap at human typing
rates. Multi-line soft-wrap is a possible v2 (see §7).

### 2.4 `screen.lpc` + `widget.lpc` — full-screen TUIs

`screen.lpc` (clonable) is the ncurses core: a virtual cell grid + damage
tracking + minimal-update renderer.

```c
void   scr_resize(int cols, int rows);      // clears; call from window_size
void   scr_clear();  void scr_invalidate(); // force full repaint next frame
void   scr_put(int x, int y, string text, mixed attr);  // attr = SGR params ("1;36") or 0
void   scr_fill(int x, int y, int w, int h, string ch, mixed attr);
void   scr_hline/scr_vline(...);  void scr_box(x, y, w, h, style, attr);
void   scr_set_cursor(int x, int y, int visible);
string scr_frame();                          // ANSI delta vs. last frame
```

Cells store `({ grapheme, sgr_params, width })`; a wide char owns its left
cell and marks the right cell as a continuation. `scr_frame()` diffs against
the previously-emitted grid row by row, coalescing changed spans into
`CUP + SGR + text` runs while tracking the current SGR state to avoid
redundant attribute churn — the standard curses algorithm, minus insert/delete
-line optimizations that don't pay at MUD line rates.

`widget.lpc` is the inheritable widget base (geometry, focus, `draw(screen)`,
`handle_key(ev)` → handled?, the on-event callback), and `app.lpc` is the
application container (screen ownership, Tab focus cycling, key/mouse
routing, Ctrl-C quit). The shipped widget set (each a clonable in `w/`,
inspired by the pterm and blessed catalogs):

| Widget | Behavior | Events |
|---|---|---|
| `label` | static (multi-line) text | — |
| `list` | scrollable single-select list | `select`, `change` |
| `table` | column-aligned rows + header, scroll + selection | `select`, `change` |
| `tree` | collapsible tree (`▸`/`▾`), ←/→/Enter fold | `select`, `change` |
| `textfield` | single-line input backed by a full readline engine | `submit` |
| `checklist` | `[x]` multi-select, Space toggles | `change` |
| `radiolist` | `(•)` single-choice group | `change` |
| `button` | `[ OK ]`, Enter/Space presses | `press` |
| `progress` | label + bar + percentage | — |
| `spinner` | braille spinner; app drives `tick()` from its own call_out | — |
| `log` | bottom-anchored scrollback pane (PgUp/PgDn) | — |

### 2.5 `print.lpc` — inline printers (stateless, inheritable)

The pterm side of the library: string builders that compose with plain
`write()` — no raw mode, no full screen — and double as rendering helpers
for widgets. `p_table(rows, opts)` (boxed, width-aware, header rule),
`p_tree(nodes)` (`├──`/`└──`), `p_bars(items)` (horizontal bar chart),
`p_spark(values)` (▁▂▃▅▇ sparkline), `p_panel(title, body)`,
`p_bullets(items)` (depth-nested), `p_header(text)` (centered banner),
`p_progress(pct, width)`, `p_info/p_success/p_warn/p_error(text)` prefix
printers, and `p_bigtext(text)` (banner letters via `/std/bitmap_font`).
All wide-char aware; tables/trees/panels accept the `TUI_BOX_*` styles.

### 2.6 `menu.lpc` — inline select/multiselect (clonable state machine)

pterm's interactive prompts, MUD-style: a prompt plus a scrolling choice
list rendered *in the normal output flow* and repainted in place with
relative cursor movement; on completion it collapses to a one-line
`? prompt: answer` record. Space toggles in multi mode, Enter accepts,
Esc/Ctrl-C aborts, long lists window with `height`. Same engine API shape
as readline: `m_begin/m_feed/m_take_output/m_result`.

### 2.7 `terminal.lpc` — the only impure module (inheritable)

Inherit into an interactive object (user/login) to run either mode:

* **Prompt mode** — `tui_readline(function done_cb, mapping opts)`: an
  enhanced `input_to` replacement: raw mode on, run the readline engine,
  restore line mode, call `done_cb(line_or_0, state)`. History is kept per
  object (`opts` can seed/limit it), giving every prompt in the mudlib
  line editing + ↑↓ history + C-r search with one call.
* **Inline prompts** — `tui_select(cb, prompt, choices, opts)` →
  `cb(index, item, state)`; `tui_multiselect(...)` →
  `cb(indexes, items, state)`; `tui_confirm(cb, prompt, default)` →
  `cb(yes, state)`. All run the menu engine (or a one-keystroke y/n loop)
  over the same char-mode plumbing as readline.
* **App mode** — `tui_open(object app)` / `tui_close()`: alternate screen,
  cursor hidden, optional mouse on; forwards decoded events to the app
  (`on_key`, `on_mouse`, `on_resize(w,h)`, `on_open(term)`, `on_close`) and
  gives it a `screen` clone to draw into.

Responsibilities: `get_char(..., I_NOECHO)` re-arm on every keystroke *inside*
the callback (so echo never flickers back on), NAWS/TTYPE request + caching
(`window_size`/`terminal_type` applies, forwarded as resize events), the
lone-ESC `call_out_walltime(0.05)` flush, and **guaranteed teardown** (restore
line mode + echo + main screen + cursor + mouse off) on close, user
disconnect, or object destruction. A safety net: if the driver reverts to
line mode behind our back, the next line of input is handled gracefully.

## 3. File layout

```
testsuite/include/tui.h                — key codes, states, macros (public contract)
testsuite/std/tui/README.md            — this document
testsuite/std/tui/ansi.lpc             — layer 0: escapes + width toolkit
testsuite/std/tui/print.lpc            — layer 0: pterm-style inline printers
testsuite/std/tui/keys.lpc             — layer 1: input decoding
testsuite/std/tui/readline.lpc         — layer 2: line editor
testsuite/std/tui/menu.lpc             — layer 2: inline select/multiselect
testsuite/std/tui/screen.lpc           — layer 2: cell grid + diff renderer
testsuite/std/tui/widget.lpc           — layer 2: widget base (inheritable)
testsuite/std/tui/w/*.lpc              — the widget set (see §2.4)
testsuite/std/tui/app.lpc              — layer 2: application container
testsuite/std/tui/terminal.lpc         — layer 3: interactive glue
testsuite/command/tuidemo.lpc          — the showcases (see top of file)
testsuite/clone/tuidemo_*.lpc          — full-screen showcase apps
testsuite/single/tests/std/tui/*.lpc   — testsuite coverage
docs/concepts/general/tui.md           — user-facing documentation
```

## 4. Testing strategy

Everything below the glue is a pure state machine, so tests are ordinary
non-interactive testsuite files:

* `tui_ansi.lpc` — sequence builders; `visible_width`/`width_slice` against
  ANSI-laden and wide-char strings.
* `tui_keys.lpc` — feed byte streams exactly as `get_char` would deliver them
  (split across calls, batched, interleaved) and assert the event stream:
  arrows, modified CSI, SS3, Alt-x, paste, mouse, lone ESC via `flush()`,
  UTF-8 text.
* `tui_readline.lpc` — script whole editing sessions as event arrays; assert
  buffer, cursor, state transitions, history navigation/search results, and
  selected render outputs (e.g. masked mode, horizontal scroll markers).
* `tui_screen.lpc` — draw, frame, assert emitted ANSI; second frame after a
  small change must touch only the damaged region; wide-char overwrite edge
  cases.
* `tui/widgets.lpc` + `tui/widgets2.lpc` — the whole widget set headless:
  navigation, events, scrolling, toggle state, and selected render output.
* `tui/print.lpc` — exact expected strings for the printers (box drawing,
  width math, scaling).
* `tui/menu.lpc` — scripted select/multiselect sessions: navigation,
  toggling, windowing, accept/abort, the collapsed answer line.

Clones are destructed at test end (DEBUGMALLOC `check_memory()` runs after
every file). The interactive glue is kept too thin to need a terminal to
verify; the demo command is the manual test.

## 5. Conventions

* Public API prefixes per module (`rl_`, `scr_`, `tui_`) — mudlibs mix these
  into objects that already have dozens of functions; no generic names.
* Events, not callbacks-into-the-decoder: lower layers *return* data; only
  `terminal.lpc` calls outward (into the app object).
* No `input_to`-style string callbacks — function pointers only.
* 0-based x,y screen coordinates, `(0,0)` top-left, x = column.
* All modules compile warning-free.
* Control characters in the edit buffer (e.g. from a bracketed paste) are
  kept verbatim in the data but rendered as nothing (they are zero-width);
  the accepted line preserves them exactly.
* The screen's rows are copy-on-write against the last emitted frame, so a
  frame's cost is proportional to the rows actually touched, not W×H.

## 6. Driver fixes bundled with this library

Five driver quirks — found by building this library and verifying it over a
live telnet connection — made a clean LPC implementation impossible; all are
fixed alongside (documented in `docs/efun/interactive/get_char.md`; the
UTF-8 tail logic is pinned by GTest `src/tests/test_strutils.cc`):

1. **BS/DEL erasure** (`comm.cc first_cmd_in_buf`): char mode zeroed
   `0x08`/`0x7f` before delivery, so the callback received `""` for
   Backspace — BS, DEL and NUL were indistinguishable. Now the literal byte
   is delivered. (Line-mode in-buffer BS editing is unchanged.)
2. **UTF-8 byte-splitting** (`comm.cc cmd_in_buf`/`first_cmd_in_buf`): char
   mode delivered one *byte* per callback, so a multi-byte character arrived
   as 2–4 invalid one-byte strings. Char-mode extraction is now UTF-8-aware:
   a complete multi-byte sequence is one callback carrying one valid
   character. Malformed bytes fall back to byte-at-a-time delivery (no
   stalls).
3. **Inbound ESC stripping** (`comm.cc on_user_input`): with `no ansi` +
   `strip before process input` (both default **on**), inbound ESC was
   rewritten to a space — arrow keys arrived as literal `[A`. The rewrite is
   an anti-ANSI-injection protection for *line-mode commands*; char mode now
   always passes ESC through (the mudlib asked for raw keystrokes).
4. **UTF-8 split across TCP segments** (`net/telnet.cc on_telnet_data` +
   `u8_incomplete_tail()` in `base/internal/strutils`): each received chunk
   was `u8_sanitize`d independently, so a multi-byte character straddling a
   segment boundary became U+FFFD before reaching the input buffer — in any
   input mode. An incomplete trailing sequence is now carried over
   (`interactive_t::u8_carry`) and prepended to the next chunk.
5. **NAWS lost at logon** (`net/telnet.cc` + `comm.cc on_user_logon`): fast
   clients answer the initial `DO NAWS` while `ip->ob` is still the master
   object, so the `window_size` apply fired on the wrong object and the size
   was unrecoverable (clients only re-send NAWS on resize). The last report
   is now cached in `interactive_t` and the apply is replayed on the user
   object at logon.

The decoder additionally copes with the old `""`-for-Backspace behavior, so
the library degrades gracefully on older drivers for everything except
non-ASCII input and escape sequences under default configs.

## 7. Deliberate scope cuts

* **Multi-line soft-wrapped editing** (readline renders one terminal row with
  horizontal scroll). The engine's render layer is isolated, so a wrap
  renderer can replace it without touching editing logic.
* **Undo** (C-_), **kill ring** beyond a single buffer, vi keymap.
* **Grapheme-cluster cursor motion** — cursor moves by codepoint; combining
  marks may take two arrow presses. (Display width is still correct.)
* **terminfo** — we emit the VT100/xterm common subset that every MUD client,
  xterm.js, and modern terminal understands; `terminal_type` is cached and
  exposed so apps can special-case, but the library does not vary output.
* **Scroll-region / insert-line diff optimizations** in the renderer.
* **Fuzzy filtering in select/multiselect** (pterm types-to-filter; our menu
  navigates only — the engine's input path has room for it).
* From the pterm/blessed catalogs, deliberately not ported: pterm's Area /
  Heatmap / theming, blessed's FileManager, Terminal, Image/Video and
  absolute-positioned Layout manager — MUD-side value didn't justify the
  surface area.
