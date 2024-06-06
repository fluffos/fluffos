---
layout: doc
title: contrib / remove_interactive.pre
---
# remove_interactive

### NAME

    remove_interactive - removes the interactive object

### SYNOPSIS

    int remove_interactive(object interactive);

### DESCRIPTION

    If the argument object is interactive and not destructed, cause it to be
    disconnected and lose interactive status. Returns 1 when the operation is
    successful.

    This function requires PACKAGE_CONTRIB to be defined in the options file.

### SEE ALSO

    exec(3), interactive(3), this_interactive(3), this_player(3)
