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

    To read and analyze the json file, use Chrome or Firefox Developer Tools
    Performance tab.

### SEE ALSO

    trace_start(3)

