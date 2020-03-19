---
layout: default
title: contrib / terminal_colour.pre
---

### NAME

    terminal_colour

### SYNOPSIS

    string terminal_colour(string, mapping, int | void, int | void);

### DESCRIPTION

    string: to be parsed for '%^KEY%^' sequences
    mappingg: 'KEY:value' pairs
    1st int: wrap after int printed symbols
    if int < 0 fillout lines with blank's

    wrap = 4:
    '12\n345'
    wrap = -4:
    '12 \n345 '

    minmwrap: 2

    2nd int: indent by int blank's
    maxindent: wrap-2

    returns wrapped and indented string with each '%^KEY%^' replaced by KEY's value
    or original string if no '%^KEY%^' sequence is found!!!
    (shoudln't it at least be wrapped and indented?)
