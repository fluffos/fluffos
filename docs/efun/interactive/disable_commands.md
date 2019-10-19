---
layout: default
title: interactive / disable_commands
---

### NAME

    disable_commands() - makes a living object non-living

### SYNOPSIS

    int disable_commands( void );

### DESCRIPTION

    Makes a living object non-living, that is, add_actions have no effects,
    livingp returns false, and, if the object is interactive, disallows the
    user  to type in commands other than for an input_to.  disable_commands
    always returns 0.

    calling disable_commands() also have side-effct of clearing all actions
    previously  added  by  other  object,  it also removes actions that was
    defined by this object.

### SEE ALSO

    enable_commands(3)

