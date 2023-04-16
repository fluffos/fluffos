---
layout: default
title: system / trace_end
---

### NAME

    trace_end - stop collecting driver trace

### SYNOPSIS

    void trace_end()

### DESCRIPTION

    Calling this function will stop an active trace and write out the result
    to the file provided by `trace_start(filename)`.

    Note: do not leave tracing enabled for too long or you will risk running
    out of memory.

### ANALYSIS

    To read and analyze the the json file,  open up Chrome's Developer Tools
    and under the Performance tab click the Load Profile icon to display the
    trace.

### SEE ALSO

    trace_start(3)

