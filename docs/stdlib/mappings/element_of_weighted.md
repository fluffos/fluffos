---
layout: default
title: stdlib / element_of_weighted
---

### NAME

    element_of_weighted() - returns a random property from a mapping weighted by its value.

### SYNOPSIS

    mixed element_of_weighted( mapping )

### DESCRIPTION

    The intent of this function is to provide a means of random selection
    of properties which are weighted to provide for most likely and
    least likely outcomes. Elements with values of higher numbers are
    more heavily weighted and therefore more likely to be returned from
    the randomization process.

    This function accepts a mapping in the form of ([ mixed property : int weight ]).
    A weight of 0 will never be chosen. Weights do not need to be in percentages,
    although some find it easier to conceptualize it that way.


### EXAMPLE

```c
mapping colors = ([ "red" : 50, "green" : 25, "blue" : 25 ]);
string result = element_weighted( colors );
// High chance of "red" being returned while lower, and equal chances
// of green and blue being returned.
```

