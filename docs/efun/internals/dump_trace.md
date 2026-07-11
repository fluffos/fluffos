---
title: internals / dump_trace
---
# dump_trace

### NAME

    dump_trace() - snapshot the current LPC call stack

### SYNOPSIS

    mixed *dump_trace();

### DESCRIPTION

    Returns an array describing the current LPC call stack, with the
    innermost (currently executing) frame last. Each element is a mapping
    describing one stack frame, with the following keys:

    "function"  The function name. For a catch frame this is "CATCH", for
                a fake frame it is "<fake>", and for a function-pointer
                frame it is a rendering of the function pointer.
    "program"   The program's filename, with a leading slash.
    "object"    The object the frame is executing in.
    "file"      The source file for the frame, with a leading slash.
    "line"      The line number within that source file.

    For real function frames, the mapping additionally carries an
    "arguments" array (the arguments the frame was called with) and, when
    present, a "locals" array (the frame's local variables).

    Returns an empty array when there is no current program or call stack.
    This is a snapshot intended for debugging and introspection.

### SEE ALSO

    call_stack(3), origin(3)
