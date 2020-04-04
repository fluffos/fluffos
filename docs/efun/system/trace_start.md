---
layout: default
title: system / trace_start
---

### NAME

    trace_start - start collecting driver trace

### SYNOPSIS

    void trace_start(filename, auto_stop_sec = 30)

### DESCRIPTION

    Call this funciton to start collecting tracing information, this include LPC
    funciton level execution information.

    Driver will stop collection and dump to `filename` on `trace_end()`, or
    after `auto_stop_sec` seconds, default to be 30.

    Note: do not leave tracing enabled for too long, otherwise you will run out of
    memory.

### SEE ALSO

    trace_end()

