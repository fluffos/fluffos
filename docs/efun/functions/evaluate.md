---
layout: default
title: functions / evaluate
---

### NAME

    evaluate() - evaluate a function pointer

### SYNOPSIS

    mixed evaluate(mixed f, ...)

### DESCRIPTION

    If f is a function, f is called with the rest of the arguments.  Other‐
    wise, f is returned.  evaluate(f, ...) is the same as (*f)(...).

