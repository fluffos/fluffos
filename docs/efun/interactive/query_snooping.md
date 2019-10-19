---
layout: default
title: interactive / query_snooping
---

### NAME

    query_snooping() - return the object than an object is snooping

### SYNOPSIS

    object query_snooping( object ob );

### DESCRIPTION

    If 'ob' (an interactive object) is snooping another interactive object,
    the snooped object is returned.  Otherwise, 0 is returned.

### SEE ALSO

    snoop(3), query_snoop(3)

