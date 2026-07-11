#ifndef TUI_H
#define TUI_H

// ---------------------------------------------------------------------------
// /std/tui — public contract.  See /std/tui/DESIGN.md.
//
// Key events are ints (special keys / modified keys) or strings (printable
// text; a whole bracketed paste arrives as one string).  Mouse reports are
// arrays: ({ TUI_EV_MOUSE, button, x, y, pressed }).
// ---------------------------------------------------------------------------

// Object paths (extension-less: the driver resolves .lpc/.c)
#define TUI_DIR        "/std/tui"
#define TUI_ANSI       "/std/tui/ansi"
#define TUI_KEYS       "/std/tui/keys"
#define TUI_READLINE   "/std/tui/readline"
#define TUI_SCREEN     "/std/tui/screen"
#define TUI_WIDGET     "/std/tui/widget"
#define TUI_APP        "/std/tui/app"
#define TUI_TERMINAL   "/std/tui/terminal"
#define TUI_PRINT      "/std/tui/print"
#define TUI_MENU       "/std/tui/menu"
#define TUI_CANVAS     "/std/tui/canvas"

// Special keys sit just above the Unicode range (max codepoint 0x10FFFF),
// so they can never collide with a real character.
#define TUI_KEY_SPECIAL   0x110000
#define TUI_KEY_UP        (TUI_KEY_SPECIAL + 1)
#define TUI_KEY_DOWN      (TUI_KEY_SPECIAL + 2)
#define TUI_KEY_RIGHT     (TUI_KEY_SPECIAL + 3)
#define TUI_KEY_LEFT      (TUI_KEY_SPECIAL + 4)
#define TUI_KEY_HOME      (TUI_KEY_SPECIAL + 5)
#define TUI_KEY_END       (TUI_KEY_SPECIAL + 6)
#define TUI_KEY_PGUP      (TUI_KEY_SPECIAL + 7)
#define TUI_KEY_PGDN      (TUI_KEY_SPECIAL + 8)
#define TUI_KEY_INSERT    (TUI_KEY_SPECIAL + 9)
#define TUI_KEY_DELETE    (TUI_KEY_SPECIAL + 10)
#define TUI_KEY_ENTER     (TUI_KEY_SPECIAL + 11)
#define TUI_KEY_TAB       (TUI_KEY_SPECIAL + 12)
#define TUI_KEY_BACKSPACE (TUI_KEY_SPECIAL + 13)
#define TUI_KEY_ESC       (TUI_KEY_SPECIAL + 14)
#define TUI_KEY_F1        (TUI_KEY_SPECIAL + 21)
#define TUI_KEY_F2        (TUI_KEY_SPECIAL + 22)
#define TUI_KEY_F3        (TUI_KEY_SPECIAL + 23)
#define TUI_KEY_F4        (TUI_KEY_SPECIAL + 24)
#define TUI_KEY_F5        (TUI_KEY_SPECIAL + 25)
#define TUI_KEY_F6        (TUI_KEY_SPECIAL + 26)
#define TUI_KEY_F7        (TUI_KEY_SPECIAL + 27)
#define TUI_KEY_F8        (TUI_KEY_SPECIAL + 28)
#define TUI_KEY_F9        (TUI_KEY_SPECIAL + 29)
#define TUI_KEY_F10       (TUI_KEY_SPECIAL + 30)
#define TUI_KEY_F11       (TUI_KEY_SPECIAL + 31)
#define TUI_KEY_F12       (TUI_KEY_SPECIAL + 32)
#define TUI_KEY_UNKNOWN   (TUI_KEY_SPECIAL + 63)

// Modifier bits, above the special-key range.
#define TUI_MOD_SHIFT     0x200000
#define TUI_MOD_ALT       0x400000
#define TUI_MOD_CTRL      0x800000
#define TUI_MODS          (TUI_MOD_SHIFT | TUI_MOD_ALT | TUI_MOD_CTRL)

// Convenience: TUI_CTRL('a') is what Ctrl-A decodes to.
#define TUI_CTRL(c)       (TUI_MOD_CTRL | (c))
#define TUI_ALT(c)        (TUI_MOD_ALT | (c))

// Array-event kinds (first element of an array event)
#define TUI_EV_MOUSE      1
#define TUI_EV_CPR        2   /* cursor position report: ({ kind, row, col }) */

// readline engine states (rl_feed() return / rl_state())
#define TUI_RL_MORE       0   /* still editing */
#define TUI_RL_DONE       1   /* line accepted (Enter) */
#define TUI_RL_ABORT      2   /* Ctrl-C */
#define TUI_RL_EOF        3   /* Ctrl-D on an empty line */

// Box styles for scr_box()
#define TUI_BOX_SINGLE    0
#define TUI_BOX_DOUBLE    1
#define TUI_BOX_ROUND     2
#define TUI_BOX_ASCII     3

#endif
