/*
 * LPC-visible surface of the source-level debugger (src/debugger/DESIGN.md).
 *
 * debugger_attached(): 1 while a debugger client is attached, else 0.
 * debug_break(): programmatic breakpoint -- stops the VM at the next
 *     instruction when a debugger is attached; an exact no-op otherwise
 *     (safe to leave in mudlib code, like JavaScript's `debugger;`).
 */
    int debugger_attached();
    void debug_break();
