---
layout: default
title: parsing / query_verb
---

### NAME

    query_verb() - return the name of the command currently being executed

### SYNOPSIS

    string query_verb( void );

### DESCRIPTION

    Give the name of the current command, or 0 if not executing from a com‐
    mand.  This function is useful when several commands (verbs) may  cause
    the  same  function  to  execute and it is necessary to determine which
    verb it was that invoked the function.

### SEE ALSO

    add_action(3)

