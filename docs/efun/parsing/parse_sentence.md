---
layout: default
title: parsing / parse_sentence
---

### NAME

    parse_sentence() - parse the command contained in the given string

### SYNOPSIS

    mixed parse_sentence(string, void | int, void | object *, void | mapping);

### DESCRIPTION

    This efun calls the driver parser and tells it to parse and execute the
    command contained in the given string. The efun may return an integer
    error code, or a string error message. If a string message is returned,
    it should be displayed to the player. The integer codes are:

    * 1: command has been processed ok
    * 0: no matching command was found, no processing done
    * -1: A matching command was found but none of its rules were matched.
    * -2: A rule made sense but all "can" or "direct" applies returned 0.

### SEE ALSO

    add_action(3)

