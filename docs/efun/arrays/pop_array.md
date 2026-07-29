---
title: arrays / pop_array
---
# pop_array

### NAME

    pop_array() - remove and return the last element of an array variable

### SYNOPSIS

    mixed pop_array(mixed *arr);

### DESCRIPTION

    Removes the last element of the array held by `arr` and returns it.

    The argument is a *variable*, not a value: `pop_array()` writes the
    shortened array back into it.  No `ref` keyword is needed at the call
    site -- the driver forms the reference itself.

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

pop_array(a);
// a is ({ 1, 2 }), b is still ({ 1, 2, 3 })
```

    Popping an empty array is not an error.  It is a no-op that returns
    undefined, the way an out-of-range slice_array() clamps rather than
    failing.

### RETURN VALUE

    Returns the element that was removed.

    On an empty array, returns undefined and leaves the variable alone.
    Because an array can legitimately contain 0, test with undefinedp()
    rather than comparing against 0 when the distinction matters:

```c
mixed *a = ({ 0 });

mixed x = pop_array(a);            // 0, undefinedp(x) == 0
mixed y = pop_array(a);            // undefined, undefinedp(y) == 1
```

    An element holding an object that has since been destructed comes back
    as undefined, matching what indexing that element would have yielded.

### EXAMPLE

```c
mixed *stack = ({ "a", "b", "c" });

while (sizeof(stack)) {
  write(pop_array(stack) + "\n");
}
// writes c, b, a

// Draining to empty leaves a usable array, not a broken one.
push_array(stack, "again");
// stack is ({ "again" })
```

    Mutating an array while iterating it is safe.  A `foreach` loop walks
    the array as it stood when the loop was entered, so every element is
    still visited even if the array empties meanwhile:

```c
mixed *a = ({ 1, 2, 3, 4 });

foreach (mixed x in a) {
  pop_array(a);
}
// the loop ran four times; a is ({})
```

### SEE ALSO

    push_array(3), shift_array(3), unshift_array(3), undefinedp(3),
    sizeof(3)
