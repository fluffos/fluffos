---
title: contrib / copy.pre
---
# copy

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

### ERRORS

    Values nested deeper than 100 levels throw "Mappings, arrays and/or
    classes nested too deep (100) for copy() -- possibly a reference loop;
    see has_cycle()". A value containing a reference loop always exceeds
    the cap; test with has_cycle(3) and clear with break_cycles(3).

### SEE ALSO

    has_cycle(3), break_cycles(3), save_object(3), restore_object(3)
