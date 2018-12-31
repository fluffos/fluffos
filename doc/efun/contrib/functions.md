---
layout: default
title: contrib / functions.pre
---

mixed \*functions(object, int default: 0);

object has NO DEFAULT!!!
int is flag (bit field)
bit 0: only function names/more info
bit 1: ???

returns
bit 0 = 0:
({ "func1", "func2", ... })

bit 0 = 1:
({
({
"func1",
num_args,
"return type",
0|"argument type" (for each argument)
}) (for each function)
=)

see variables(3)
