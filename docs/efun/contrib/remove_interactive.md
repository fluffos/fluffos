---
layout: doc
title: contrib / remove_interactive.pre
---
# remove_interactive

### NAME

    remove_interactive() - destructs and removes the interactive object.

### SYNOPSIS

    int remove_interactive(object interactive);

### DESCRIPTION

    makes sure object isn't interactive

    returns 0 if object is either destructed or not interactive to begin with
    1 otherwise
