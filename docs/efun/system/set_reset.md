---
layout: default
title: system / set_reset
---

### NAME

    set_reset - modify the time until reset on an object

### SYNOPSIS

    varargs void set_reset( object ob, int time );

### DESCRIPTION

    Sets  the  time  until  reset  on  'ob' to 'time' seconds from now.  If
    'time' is omitted, the driver's normal reset time  setting  formula  is
    applied to 'ob', that is,

    reset time = current_time + reset_time / 2 + random(reset_time / 2)

### SEE ALSO

    reset(4)

