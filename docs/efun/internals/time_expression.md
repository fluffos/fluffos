---
layout: default
title: internals / time_expression
---

### NAME

    time_expression()  -  return the amount of real time that an expression
    took

### SYNOPSIS

    int time_expression( mixed expr );

### DESCRIPTION

    Evaluate <expr>.  The amount of real time that passed during the evalu‐
    ation  of  <expr>,  in microseconds, is returned.  The precision of the
    value is not necessarily 1 microsecond; in fact, it  probably  is  much
    less precise.

### SEE ALSO

    rusage(3), function_profile(3), time(3)

