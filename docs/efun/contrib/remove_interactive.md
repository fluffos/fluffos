---
layout: default
title: contrib / remove_interactive.pre
---

### NAME

    remove_interactive

### SYNOPSIS

    int remove_interactive(object);

### DESCRIPTION

    makes sure object isn't interactive

    returns 0 if object is either destructed or not interactive to begin with
    1 otherwise
