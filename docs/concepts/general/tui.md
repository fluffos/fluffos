---
title: general / tui
---
# The LPC TUI library (/std/tui)

The testsuite mudlib ships a terminal-UI toolkit written in pure LPC, layered
like `readline` + `ncurses` + `pterm`: line editing, history and incremental
search for the prompt line, inline select/multiselect/confirm prompts and
pretty-printers for ordinary output, plus a screen buffer, diff renderer and
widget set for full-screen applications.  It lives in `testsuite/std/tui/`
and is portable to any FluffOS mudlib (copy the directory plus
`include/tui.h`).

The architecture document is
[`testsuite/std/tui/README.md`](https://github.com/fluffos/fluffos/blob/master/testsuite/std/tui/README.md);
this page is the user view.

## Prompt-line editing (the readline replacement)

Inherit `/std/tui/terminal` into your user object (it also provides the
`window_size` / `terminal_type` applies), then replace `input_to()` calls
with:

```c
void get_command() {
    tui_readline((: got_command :), ([ "prompt": "> " ]));
}

void got_command(mixed line, int state) {
    if (state != TUI_RL_DONE) return;      // TUI_RL_ABORT (^C) / TUI_RL_EOF (^D)
    do_command(line);
    get_command();
}
```

Every prompt now has Emacs/readline editing: cursor motion (arrows,
Ctrl-A/E/B/F, word motion Alt-B/F or Ctrl-arrows), kill/yank
(Ctrl-K/U/W/Y, Alt-D), Ctrl-T transpose, ↑/↓ history, Ctrl-R/Ctrl-S
incremental history search, Tab completion (pass a `"completer"` function in
the options), bracketed paste, masked input (`"masked": 1` for passwords),
wide-character (CJK) aware rendering with horizontal scrolling, and live
terminal-resize handling via NAWS.  History persists per user object across
calls.

## Inline prompts and printers (the pterm layer)

Three prompt helpers run in the normal output flow — no full-screen mode:

```c
tui_select((: chose :), "Pick a class:", ({ "warrior", "mage", "thief" }));
// -> chose(int index, string item, int state)
tui_multiselect((: picked :), "Skills:", skills, ([ "height": 5 ]));
// -> picked(int *indexes, string *items, int state)
tui_confirm((: sure :), "Delete the character?", 0);
// -> sure(int yes, int state)
```

Arrows navigate, Space toggles, Enter accepts, Esc/Ctrl-C aborts; on
completion the list collapses to a one-line `? prompt: answer` record.

`/std/tui/print` is a set of stateless printers that compose with `write()`:
`p_table()` (boxed, width-aware), `p_tree()`, `p_bars()` (bar chart),
`p_spark()` (sparkline), `p_panel()`, `p_bullets()`, `p_header()`,
`p_progress()`, `p_info/p_success/p_warn/p_error()` and `p_bigtext()`
(banner letters via `/std/bitmap_font`).  Run `tuidemo print` to see them
all.

Charts are built on `/std/tui/canvas`, a braille dot canvas (2×4 dots per
cell, the blessed-contrib technique): `p_chart()` renders multi-series
braille line charts with a y-axis and coloured legend, `p_vbars()` draws
vertical bars with eighth-block partial tops, and `p_heatmap()` renders a
2D matrix as 256-colour cells.  The same engine powers the `chart` widget
(`add_point()` rolling history — see the dashboard's traffic graph).  Run
`tuidemo charts` to see them.

## Full-screen applications

Inherit `/std/tui/app`, position widgets in `on_layout()`, and open the app
with `tui_open()` from the user object:

```c
// my_app.lpc
inherit TUI_APP;
private object lst;

void on_open(object term, int w, int h) {
    if (!lst) {
        lst = app_add(new("/std/tui/w/list"));
        lst->set_items(({ "one", "two", "three" }));
        lst->set_on_event((: on_event :));
    }
    ::on_open(term, w, h);
}

void on_layout(int w, int h) {
    app_screen()->scr_clear();
    app_screen()->scr_box(0, 0, w, h, TUI_BOX_ROUND, "36");
    lst->set_geometry(2, 1, w - 4, h - 2);
}

// somewhere in the user object:  tui_open(new("/path/to/my_app"));
```

The terminal switches to the alternate screen with the cursor hidden; the app
receives decoded key events (`on_key`), optional SGR mouse events
(`on_mouse`, pass `(["mouse": 1])` to `tui_open`), and resize events
(`on_resize`).  Tab/Shift-Tab cycle focus, Ctrl-C always quits, and
`app_quit()` restores the terminal.

Shipped widgets (`/std/tui/w/`, modeled on the pterm and blessed sets):
`label`, `list`, `table` (columns + header + selection), `tree`
(collapsible), `textfield` (a full readline engine per field), `checklist`,
`radiolist`, `button`, `progress`, `spinner`, `log` (bottom-anchored
scrollback pane), and `chart` (braille line chart with rolling history).
The widget base class (`/std/tui/widget`) makes new widgets ~30 lines;
`tuidemo dashboard` and `tuidemo form` show most of the set in action.

## The layers (use them à la carte)

| Module | What it is |
|---|---|
| `/std/tui/ansi` | escape-sequence builders; `visible_width()` (ANSI-blind, wide-char-aware), `wslice()`, `wpad()` |
| `/std/tui/canvas` | braille dot canvas: sub-cell lines/plots for charts |
| `/std/tui/print` | pterm-style inline printers: tables, trees, charts, heatmaps, panels, banners |
| `/std/tui/keys` | keystroke decoder: `get_char()` byte stream → key events (CSI/SS3, modifiers, UTF-8, bracketed paste, SGR mouse) |
| `/std/tui/readline` | the line-editor engine — a pure state machine, usable headless |
| `/std/tui/menu` | the inline select/multiselect engine behind `tui_select()` |
| `/std/tui/screen` | virtual cell grid + minimal-diff frame renderer (the ncurses core) |
| `/std/tui/widget`, `/std/tui/w/*`, `/std/tui/app` | widget protocol, the widget set, application container |
| `/std/tui/terminal` | the only impure module: `get_char` loop, NAWS/TTYPE, ESC timeout, teardown |

Everything below `terminal` is side-effect-free and covered by the LPC
testsuite (`single/tests/std/tui/`).  Key-event constants and readline states
are in `include/tui.h` (`TUI_KEY_*`, `TUI_MOD_*`, `TUI_CTRL(c)`,
`TUI_RL_*`).

## Try it

Boot the testsuite mudlib and connect with any terminal:

```bash
./driver testsuite/etc/config.test &
telnet localhost 4000
> tuidemo            # readline demo: editing, history, ^R search, Tab completion
> tuidemo select     # inline select -> multiselect -> confirm chain
> tuidemo print      # the inline printers
> tuidemo charts     # vertical bars, braille line chart, heatmap
> tuidemo app        # minimal full-screen demo
> tuidemo dashboard  # animated spinner/progress/sparkline/table/log
> tuidemo form       # textfield, radio group, checkboxes, buttons
```

## Requirements & caveats

* Works over raw telnet, the websocket `telnet` subprotocol, and the WASM
  bridge.  The websocket `ascii` subprotocol has no negotiation layer
  (no char mode / echo control), so the library cannot run on it.
* A client that never answers NAWS is assumed to be 80×24.
* The driver must not be started with both `no ansi` and
  `strip before process input` *acting on char mode* — since 2026 the driver
  keeps ESC intact in char mode regardless of those options, so any current
  build is fine.
* Rendering emits the VT100/xterm common subset (CSI cursor addressing, SGR,
  alternate screen, bracketed paste, SGR mouse) — understood by every modern
  terminal, MUD client and xterm.js.  `tui_term()` exposes the negotiated
  terminal type if an app wants to special-case.
