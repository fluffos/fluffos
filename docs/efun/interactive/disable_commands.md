---
title: interactive / disable_commands
---
# disable_commands

### NAME

    disable_commands() - makes a living object non-living

### SYNOPSIS

    void disable_commands();

### DESCRIPTION

    Makes a living object non-living, that is, add_actions have no effects,
    livingp returns false, and, if the object is interactive, disallows the
    user to type in commands other than for an input_to.

    calling disable_commands() also has the side-effect of clearing all actions
    previously  added  by  other  object,  it also removes actions that was
    defined by this object.

### SEE ALSO

    enable_commands(3)

