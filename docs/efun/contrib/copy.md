---
layout: default
title: contrib / copy.pre
---

### NAME

    mixed copy(mixed arg) - return a deep copy of an array, buffer, class, or
    mapping

### SYNOPSIS

    mixed copy(mixed arg);

### DESCRIPTION

    arg is either one of:

    - array
    - buffer (if compiled into driver)
    - class
    - mapping

    returns deep copy

    This is particularly useful when you wish to have data that is passed
    by reference, but do not want to alter the original.
