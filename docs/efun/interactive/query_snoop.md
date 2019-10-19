---
layout: default
title: interactive / query_snoop
---

### NAME

    query_snoop() - return the snooper of an interactive object

### SYNOPSIS

    object query_snoop( object ob );

### DESCRIPTION

    If 'ob' (an interactive object) is being snooped by another interactive
    object, the snooping object is returned.  Otherwise, 0 is returned.

### SEE ALSO

    snoop(3), query_snooping(3)

