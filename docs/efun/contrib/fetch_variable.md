---
layout: default
title: contrib / fetch_variable.pre
---

mixed fetch_variable(string, object | void);

object defaults to this_object()
string is name of global variable (not private!!!) in object

returns object.string

POSSIBLE SECURITY HAZARD!!!!

see store_variable.3
