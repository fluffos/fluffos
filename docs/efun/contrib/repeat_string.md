---
layout: default
title: contrib / repeat_string.pre
---

### NAME

    repeat_string() - repeat a string a certain number of times.

### SYNOPSIS

    string repeat_string(string str, int repeats);

### DESCRIPTION

    returns a new string

    - repeats <= 0: ""
    - repeats > 0 original string repeated either int times or the maximum
      number of times without getting greater than the maximum string length

    maxstrlen = 5:
    repeat_string("ab", 3) = "abab"

    maxstrlen >=6:
    repeat_string("ab", 3) = "ababab"
