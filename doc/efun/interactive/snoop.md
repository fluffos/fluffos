---
layout: default
title: interactive / snoop
---

### NAME

    snoop() - snoop an interactive user

### SYNOPSIS

    varargs object snoop( object snooper, object snoopee );

### DESCRIPTION

    When   both  arguments  are  used,  begins  snooping  of  'snoopee'  by
    'snooper'.  If the second argument is omitted, turns off  all  snooping
    by  'snoopee'.   Security  for  snoop()  is  normally  controlled  by a
    simul_efun.  snoop() returns successful in the single-argument case.  A
    return of 0 indicates failure.

### SEE ALSO

    query_snoop(3), query_snooping(3)
