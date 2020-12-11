---
layout: default
title: stdlib / reduce
---

### NAME

    reduce - Inspired by JavaScript's Array.reduce()

### SYNOPSIS

    varargs mixed reduce( mixed *elements, mixed fun, mixed initial, mixed args... )

### DESCRIPTION

    Iterate over an <arr>, passing each element to a function <fun>.
    The function should return a new value which becomes the new
    total. Function <fun> can be a function or a string. If it is a
    string, it will call the named function in the calling object
    (via previous_object()).

    Parameters:

    (required) mixed *elements - the array to be worked upon
    (required) function | string fun - the reducer function
    (optional) mixed initial - the initial value to set
    (optional) mixed args... - additional arguments to pass to <fun>

    The following are passed as arguments to <fun> at every iteration.

    (required) mixed total - The <initial> value, or the previously returned value of the function
    (required) mixed currentValue - The value of the current element
    (optional) int currentIndex - The array index of the current element
    (optional) mixed *arr - The array the current element belongs to
    (optional) mixed args... - Additional arguments

    The function should return what will become the new total.

### EXAMPLE
```c
reduce( ({ 1, 2, 3, 4 }), function( int total, int current ) {
   return total + current ;
}) ;
//Result: 10
```

```c
reduce( ({ 1, 2, 3, 4 }), function( int total, int current ) {
   return total + current ;
}, 10) ;
//Result: 20
```

```c
result7 = reduce( explode("FluffOS", ""), function( int total, int current, int idx, int* arr ) {
    return total + arr[sizeof(arr) - idx - 1] ;
}, "") ;
//Result: "SOffulF"
```

```c
reduce( ({ "north", "south", "east", "west" }), function( string total, string current, int idx, string* arr, string conjunction ) {
    if( idx == 0 ) return current ;

    total += ", " ;

    if( idx == sizeof( arr ) - 1 ) total += conjunction + " " ;

    return total + current ;
}, "", "or") ;
//Result: "north, south, east, or west"
```
