---
title: arrays / unshift_array
---
# unshift_array

### NAME

    unshift_array() - prepend a value to an array variable

### SYNOPSIS

    int unshift_array(mixed *arr, mixed value);

### DESCRIPTION

    Inserts `value` at the front of the array held by `arr`, shifting the
    existing elements up one position, and returns the new element count.

    The first argument is a *variable*, not a value: `unshift_array()`
    writes the resulting array back into it.  No `ref` keyword is needed
    at the call site -- the driver forms the reference itself.

    Anything assignable holding an array may be the target: a local, a
    global, an element of another array, a mapping value, a `ref`
    parameter, or a `foreach` loop variable declared `ref`.  A function
    call result or a range (`arr[0..2]`) is not assignable and is
    rejected at compile time.

    The array itself is changed, not replaced, so everything holding that
    array sees the new element:

```c
mixed *a = ({ 2, 3 });
mixed *b = a;

unshift_array(a, 1);
// both a and b are ({ 1, 2, 3 })
```

    This is what distinguishes it from `a = ({ 1 }) + a`, which builds a new
    array and rebinds only `a`.

### RETURN VALUE

    Returns the number of elements in the array after the insert.

    Errors if the result would exceed the driver's maximum array size
    (the `max_array_size` runtime configuration value); the array is left
    unchanged in that case.

### EXAMPLE

```c
mixed *stack = ({ "b", "c" });

unshift_array(stack, "a");         // returns 3
// stack is ({ "a", "b", "c" })

// Building a most-recent-first list.
mixed *recent = ({});
unshift_array(recent, "third");
unshift_array(recent, "second");
unshift_array(recent, "first");
// recent is ({ "first", "second", "third" })
```

### SEE ALSO

    shift_array(3), push_array(3), pop_array(3), sizeof(3), allocate(3)
