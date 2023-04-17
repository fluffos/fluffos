---
layout: default
title: system / trace_start
---

### NAME

    trace_start - start collecting driver trace

### SYNOPSIS

    void trace_start(string filename, int auto_stop_sec = 30)

### DESCRIPTION

    Calling this function will start collecting tracing information from the
    driver. This includes LPC function level execution information.

    The trace collection will stop and write the json data to  `filename` on
    `trace_end()`  or after  `auto_stop_sec`  seconds,  which defaults to 30 
    seconds.

    Note: do not leave tracing enabled for too long or you will risk running
    out of memory.

### ANALYSIS

    To read and analyze the json file, use Chrome or Firefox Developer Tools
    Performance tab.

### SEE ALSO

    trace_end(3)

