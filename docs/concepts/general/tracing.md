---
title: Tracing and Performance Profiling
---
# Tracing and Performance Profiling

FluffOS includes a comprehensive tracing system for profiling LPC code execution and analyzing performance bottlenecks. The tracing system generates detailed timeline data compatible with Chrome DevTools and Firefox Developer Tools.

## Overview

The FluffOS tracing system allows you to:

- **Profile LPC function execution** - See exactly which functions are called and how long they take
- **Identify performance bottlenecks** - Find slow code paths and optimization opportunities
- **Analyze call patterns** - Understand execution flow and call hierarchies
- **Visualize timelines** - Use browser DevTools for intuitive performance analysis
- **Debug complex systems** - Track execution across multiple objects and function calls

## How Tracing Works

While a trace is running, FluffOS records timestamped begin/end events for:

- LPC function calls and efun calls
- Object loads and file compilation
- `catch` evaluation, file I/O, and apply-cache lookups

Each event carries its duration, its nesting (caller/callee) relationship, and
the relevant object and function names. The data is saved in the Chrome Trace
Event Format (JSON), which can be opened directly in browser developer tools.

The instrumentation is always compiled in and only records while a trace is
active, so no special build is required — `trace_start()` and `trace_end()` are
ordinary efuns available in normal builds.

## Basic Usage

### Starting a Trace

```c
// Start tracing; the trace is written after the timeout (default 10 seconds)
trace_start("/log/performance.json");

// Start tracing with a custom timeout (30 seconds)
trace_start("/log/performance.json", 30);
```

The timeout must be between 0 and 300 seconds; a value outside that range
raises an error. `trace_end()` flushes the trace early, before the timeout.

### Stopping a Trace

```c
// Manually stop tracing before timeout
trace_end();
```

### Complete Example

```c
void cmd_profile(string arg) {
    if (!wizardp(this_player())) {
        write("Permission denied.\n");
        return;
    }

    if (arg == "start") {
        trace_start("/log/trace_" + time() + ".json", 60);
        write("Tracing started for 60 seconds.\n");
        write("Use 'profile stop' to end early.\n");
    }
    else if (arg == "stop") {
        trace_end();
        write("Tracing stopped. Check /log/ for trace file.\n");
    }
    else {
        write("Usage: profile <start|stop>\n");
    }
}
```

## Analyzing Trace Data

### Using Chrome DevTools

1. **Start and collect a trace:**

   ```c
   trace_start("/log/my_trace.json", 30);
   // Run the code you want to profile
   trace_end();
   ```

2. **Download the trace file** from your MUD server

3. **Open Chrome DevTools:**
   - Press F12 to open DevTools
   - Go to the "Performance" tab
   - Click the upload button (↑) in the toolbar
   - Select your trace JSON file

4. **Analyze the results:**
   - **Timeline view** - See function execution over time
   - **Bottom-Up view** - Functions sorted by total time spent
   - **Call Tree view** - Hierarchical execution structure
   - **Event Log** - Chronological list of all events

### Using Firefox Developer Tools

1. **Collect trace data** (same as Chrome)

2. **Open Firefox DevTools:**
   - Press F12 to open DevTools
   - Go to the "Performance" tab
   - Click "Load a profile from file"
   - Select your trace JSON file

3. **Analyze** using similar views as Chrome

## Practical Profiling Scenarios

### Profiling Slow Commands

```c
// In a command that seems slow
void cmd_process_inventory() {
    // Start profiling
    trace_start("/log/inventory_profile.json", 10);

    // Run the slow operation
    process_all_items(all_inventory(environment(this_player())));

    // Stop profiling
    trace_end();

    write("Profiling complete. Check /log/inventory_profile.json\n");
}
```

### Profiling Combat Rounds

```c
// In combat manager
void profile_combat_round() {
    static int profiling = 0;

    if (!profiling) {
        trace_start("/log/combat_round.json", 5);
        profiling = 1;
        call_out("stop_profiling", 5);
    }

    // Normal combat round processing
    execute_combat_round();
}

void stop_profiling() {
    trace_end();
    log_file("debug", "Combat profiling complete.\n");
}
```

### Profiling Heart Beat Performance

```c
// In a room or area manager
void enable_heartbeat_profiling() {
    trace_start("/log/heartbeat_" + file_name(this_object()) + ".json", 60);

    write("Profiling heartbeats for 60 seconds...\n");
    call_out("finish_profiling", 55);
}

void finish_profiling() {
    trace_end();
    write("Heartbeat profiling complete.\n");
}
```

## Understanding Trace Output

### Trace File Structure

The trace file uses the JSON Array form of the Chrome Trace Event Format — a
plain array of event objects (no enclosing `traceEvents` wrapper). Browser dev
tools accept this form directly:

```json
[
  {"pid":1,"tid":1,"ts":1234567890,"dur":5,"ph":"B","cat":"lpc.function","name":"create","args":{"object":"/std/object"}},
  {"pid":1,"tid":1,"ts":1234567895,"dur":0,"ph":"E","cat":"lpc.function","name":"create"}
]
```

### Event fields

- **`ph`** — phase: `B` (begin) and `E` (end) bracket a call
- **`ts`** — timestamp in microseconds
- **`dur`** — duration in microseconds
- **`cat`** — event category (e.g. LPC function, efun, object load)
- **`name`** / **`args`** — the traced name and associated metadata

The nesting of begin/end events reconstructs the call hierarchy.

## Performance Tips

### Memory Usage

A trace accumulates one begin and one end event per recorded call, so its memory
and file size grow with the number of calls — that is, with both the duration
and how busy the system is. Prefer short, targeted traces:

```c
// Good: short duration, specific scenario
trace_start("/log/login.json", 5);
test_login_sequence();
trace_end();
```

The timeout is capped at 300 seconds (5 minutes); a longer value raises an
error, so there is no way to leave a trace running indefinitely. Even within
that limit, a long trace on a busy system produces a large file.

### Trace Duration Guidelines

| Scenario | Suggested Duration |
|----------|---------------------|
| Single command | 1-5 seconds |
| Combat round | 5-10 seconds |
| Login sequence | 10-30 seconds |
| General profiling | 30-60 seconds |
| Load testing | up to a few minutes (max 300s) |

### Minimizing Overhead

Tracing adds overhead in proportion to how many calls are recorded, and it only
runs while a trace is active. To keep the impact small:

1. **Profile specific scenarios** - Don't trace everything
2. **Use appropriate durations** - Longer isn't always better
3. **Avoid production profiling** - Profile on test servers when possible
4. **Stop traces promptly** - Don't leave them running

## Advanced Usage

### Conditional Tracing

```c
// Only trace for specific players
void conditional_trace(object player) {
    if (player->query_property("enable_tracing")) {
        string file = sprintf("/log/trace_%s_%d.json",
                            player->query_name(),
                            time());
        trace_start(file, 30);
        player->set_property("trace_active", 1);
        tell_object(player, "Tracing enabled for 30 seconds.\n");
    }
}
```

### Automated Performance Testing

```c
// Performance regression testing
void run_performance_tests() {
    mapping results = ([]);
    string tests = ({ "combat", "inventory", "parser", "movement" });

    foreach(string test in tests) {
        string tracefile = "/log/test_" + test + ".json";

        // Start trace
        trace_start(tracefile, 10);

        // Run test
        call_other("/tests/" + test, "run_test");

        // Stop trace
        trace_end();

        // analyze_trace() here is your own helper — the driver does not
        // analyze traces; you inspect the JSON in browser dev tools or a
        // custom parser.
        results[test] = analyze_trace(tracefile);
    }

    return results;
}
```

### Integration with Admin Commands

```c
// In your admin command handler
void cmd_trace(string arg) {
    string *parts;

    if (!adminp(this_player())) {
        return notify_fail("Access denied.\n");
    }

    if (!arg || arg == "") {
        write("Usage:\n");
        write("  trace start [duration] - Start tracing\n");
        write("  trace stop             - Stop tracing\n");
        write("  trace status           - Check trace status\n");
        return;
    }

    parts = explode(arg, " ");

    switch(parts[0]) {
        case "start":
            int duration = sizeof(parts) > 1 ? to_int(parts[1]) : 30;
            string file = sprintf("/log/admin_trace_%s_%d.json",
                                this_player()->query_name(),
                                time());
            trace_start(file, duration);
            write(sprintf("Tracing started for %d seconds.\n", duration));
            write("Output: " + file + "\n");
            break;

        case "stop":
            trace_end();
            write("Tracing stopped.\n");
            break;

        case "status":
            // Check if tracing is active (you'll need to track this)
            write("Trace status: " +
                  (query_property("trace_active") ? "ACTIVE" : "INACTIVE") + "\n");
            break;

        default:
            write("Unknown trace command.\n");
    }
}
```

## Interpreting Results

### Common Performance Issues

**1. Hot Spots - Functions consuming most time:**

- Look at Bottom-Up view sorted by "Self Time"
- Focus on functions with >10% of total time
- Consider optimization or caching

**2. Excessive Calls - Functions called too frequently:**

- Check call counts in the statistics
- Look for unnecessary recalculations
- Consider caching or memoization

**3. Deep Call Stacks - Excessive function nesting:**

- Review Call Tree view
- Look for deeply nested calls (>10 levels)
- Consider refactoring to reduce complexity

**4. Blocking Operations - Long-running single functions:**

- Find long bars in timeline view
- These are good candidates for optimization
- Consider breaking into smaller operations

### Example Analysis

```
Timeline shows:
  process_room_items(): 450ms
    ├─ check_item(): 180ms (called 100 times)  ← OPTIMIZE THIS
    ├─ update_inventory(): 150ms
    └─ apply_effects(): 120ms

Recommendation: Cache item checks or reduce frequency
```

## Debugging with Traces

### Finding Unexpected Calls

Traces help identify:

- Functions being called when they shouldn't
- Missing function calls
- Incorrect call ordering
- Recursive loops

### Comparing Traces

To find regressions:

1. Create baseline trace of working code
2. Make changes
3. Create new trace
4. Compare execution times and call patterns

## See Also

- [trace_start](../../efun/system/trace_start) — start tracing
- [trace_end](../../efun/system/trace_end) — stop tracing
- [trace](../../efun/internals/trace) — per-execution debug tracing (separate feature)
- [traceprefix](../../efun/internals/traceprefix) — set the debug-trace prefix
- [dump_trace](../../efun/internals/dump_trace) — return the current LPC call stack

## References

- [Chrome Trace Event Format](https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/)
- [Chrome DevTools Performance](https://developer.chrome.com/docs/devtools/performance/)
- [Firefox Performance Tools](https://firefox-source-docs.mozilla.org/devtools-user/performance/)

## Notes

- Tracing is available in normal builds; no DEBUG build or special configuration is required
- Trace files can be large (1MB+ for busy systems)
- Use trace analysis to guide optimization efforts, not guesswork
- Profile before and after optimizations to measure improvement
