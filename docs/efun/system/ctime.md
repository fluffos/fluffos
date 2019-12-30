---
layout: default
title: system / ctime
---

### NAME

    ctime() - return a time string

### SYNOPSIS

    string ctime( int clock | void );

### DESCRIPTION

    Gives  a  nice  string  with  current  date and time, with the argument
    'clock' that is the number of seconds since 1970.
    
    If no arguments are provided, it will use the current time.

### SEE ALSO

    time(3), time_expression(3), localtime(3), uptime(3)

