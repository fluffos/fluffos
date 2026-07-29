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

    Only the variable passed in is updated.  Other variables holding the
    same array keep the array they already had:

```c
mixed *a = ({ 1, 2, 3 });
mixed *b = a;

push_array(a, 4);
// a is ({ 1, 2, 3, 4 }), b is still ({ 1, 2, 3 })
```

    This is the same behaviour as `a += ({ 4 })`, and differs from
    shuffle(), which reorders one array in place so that every holder of
    it observes the change.

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

    Mutating an array while iterating it is safe.  A `foreach` loop walks
    the array as it stood when the loop was entered, so the appended
    elements are not visited:

```c
mixed *a = ({ 1, 2, 3 });

foreach (mixed x in a) {
  push_array(a, x * 10);
}
// the loop ran three times; a is ({ 1, 2, 3, 10, 20, 30 })
```

### SEE ALSO

    pop_array(3), shift_array(3), unshift_array(3), member_array(3),
    sizeof(3), allocate(3)
