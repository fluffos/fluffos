---
title: arrays / push_array
---
# push_array

### NAME

    push_array() - append a value to an array variable

### SYNOPSIS

    int push_array(mixed *arr, mixed value);

### DESCRIPTION

    Appends `value` to the end of the array held by `arr` and returns the
    new element count.

    The first argument is a *variable*, not a value: `push_array()` writes
    the resulting array back into it.  No `ref` keyword is needed at the
    call site -- the driver forms the reference itself, the same way
    sscanf() does for its trailing arguments.

    Anything assignable holding an array may be the target: a local, a
    global, an element of another array, a mapping value, a `ref`
    parameter, or a `foreach` loop variable declared `ref`.  A function
    call result or a range (`arr[0..2]`) is not assignable and is
    rejected at compile time.

    The array itself is changed, not replaced, so everything holding that
    array sees the new element -- exactly as adding a key to a mapping is
    seen by every holder of that mapping:

```c
mixed *a = ({ 1, 2, 3 });
mixed *b = a;

push_array(a, 4);
// both a and b are ({ 1, 2, 3, 4 })
```

    This is what distinguishes `push_array(a, 4)` from `a += ({ 4 })`.  The
    `+=` builds a **new** array and rebinds `a` to it, leaving every other
    holder on the original; `push_array()` reshapes the array `a` names.
    Both leave `a` with the same contents, and that is where the similarity
    ends.

### RETURN VALUE

    Returns the number of elements in the array after the append.

    Errors if the result would exceed the driver's maximum array size
    (the `max_array_size` runtime configuration value); the array is left
    unchanged in that case.

### EXAMPLE

```c
mixed *queue = ({});

push_array(queue, "first");        // returns 1
push_array(queue, "second");       // returns 2
// queue is ({ "first", "second" })

// Growing a mapping value in place.
mapping m = ([ "items": ({}) ]);
push_array(m["items"], "sword");
// m is ([ "items": ({ "sword" }) ])
```

    Appending inside a `foreach` cannot extend the loop: the element count
    is taken when the loop is entered, so an append can never turn into an
    endless loop.  The appended elements are still added to the array, and
    every holder of it sees them -- they are simply not visited by this
    loop.

```c
mixed *a = ({ 1, 2, 3 });

foreach (mixed x in a) {
  push_array(a, x * 10);
}
// the loop ran three times; a is ({ 1, 2, 3, 10, 20, 30 })
```

    Removing elements from the array a `foreach` is walking is a different
    matter -- see pop_array(3).

### SEE ALSO

    pop_array(3), shift_array(3), unshift_array(3), member_array(3),
    sizeof(3), allocate(3)
