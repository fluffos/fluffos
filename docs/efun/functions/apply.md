---
layout: default
title: functions / apply
---

### NAME

    apply() - evaluate a function pointer

### SYNOPSIS

    mixed apply(mixed f, mixed *args)

### DESCRIPTION

    If  f is a function, f is called with the array 'args' expanded as it's
    arguments.  For example,

    apply(f, ({ 1, 2, 3 }) ) is the same as evaluate(f, 1, 2, 3);

