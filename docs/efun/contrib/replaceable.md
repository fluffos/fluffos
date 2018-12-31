---
layout: default
title: contrib / replaceable.pre
---

int replaceable(object, void | string \*);

string \* defaults to ({ "create", "\_\_INIT" }) and contains a list of functions which
may be ignored for checking

checks if object defines any functions itself (beside create and \_\_INIT)

returns 1 if no and object is not simul_efun object and not ???
0 otherwise
