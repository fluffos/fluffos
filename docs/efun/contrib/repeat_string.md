---
layout: default
title: contrib / repeat_string.pre
---

string repeat_string(string, int);

returns a new string

- int <= 0: ""
- int > 0 original string repeated either int times or the maximum number of
  times without getting greater than the maximum string length

maxstrlen = 5:
repeat_string("ab", 3) = "abab"

maxstrlen >=6:
repeat_string("ab", 3) = "ababab"
