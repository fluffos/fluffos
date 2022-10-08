---
layout: default
title: contrib / terminal_colour.pre
---

### NAME

    terminal_colour() - return a wrapped and indented string where the driver
    substitutes %^KEY%^ in the string with the matching VALUE of KEY in the
    supplied mapping.

### SYNOPSIS

    string terminal_colour(string, mapping, int | void, int | void);

### DESCRIPTION

    string: to be parsed for '%^KEY%^' sequences
    mapping: 'KEY:value' pairs
    1st int: wrap after int printed symbols
    if int < 0 fillout lines with blank's

    wrap = 4:
    '12\n345'
    wrap = -4:
    '12 \n345 '

    minwrap: 2

    2nd int: indent by int blanks
    maxindent: wrap-2

    returns wrapped and indented string with each '%^KEY%^' replaced by KEY's value
    or original string if no '%^KEY%^' sequence is found
    (shouldn't it at least be wrapped and indented?)
