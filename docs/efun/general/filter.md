---
layout: doc
title: general / filter
---
# filter

### NAME

    filter

### SYNOPSIS

    mixed filter(string|mapping|mixed* source,
                 string filter_function,
                 object|string ob,
                 mixed *extra...)
    mixed filter(string|mapping|mixed* source,
                 function f,
                 mixed *extra... )

### DESCRIPTION

    Returns a new string, array or mapping with the same elements as 'source'
    whose elements pass the test implemented by the filter function.

    A string is considered an array of integers, so the filter function is
    called with the integer value of each character in the string.

    The filter function must accept at least one argument, the element to be
    tested and return a truthy value if the element should be included in the
    result, or a falsy value if it should be excluded.

    In the case of a mapping, filter will pass the key and value to the filter
    function.

    Any additional arguments given to filter() will be passed to the filter
    function after the required arguments.

    If the first syntax is used, the filter function is performed as a
    call_other. If ob is a string, it is assumed to be the filename of an
    object to load and call the filter function on.

### EXAMPLES
Using function pointers:
```c
string s = "Hello, this is a string.";
s = filter(s, (: $1 != ' ' :));
// s == "Hello,thisisastring."

int *a = ({ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 });
a = filter(a, (: $1 % 2 == 0 :));
// a == ({ 2, 4, 6, 8, 10 })

mapping m = ([ "a": 1, "b": 2, "c": 3, "d": 4, "e": 5 ]);
m = filter(m, (: $1[1] % 2 == 0 :));
// m == ([ "b": 2, "d": 4 ])

not_me = filter(users(), (: $1 != $2 :), this_player());
// not_me == all users except this_player()
```
Using strings:
```c
int filter_space(int elem) return (elem != ' ');
s = filter(s, "filter_space", this_object());
// s == "Hello,thisisastring."

int filter_even(int elem) return (elem % 2 == 0);
a = filter(a, "filter_even", this_object());
// a == ({ 2, 4, 6, 8, 10 })

int filter_even(string key, int value) return (value % 2 == 0);
m = filter(m, "filter_even", this_object());
// m == ([ "b": 2, "d": 4 ])

int filter_not_me(object elem, object tp) return (elem != tp);
not_me = filter(users(), "filter_not_me", this_object(), this_player());
// not_me == all users except this_player()
```

### NOTE

    The efuns filter_mapping() and filter_array() are aliases for filter().
