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

    The array itself is changed, not replaced, so everything holding that
    array sees the element go -- exactly as map_delete() is seen by every
    holder of a mapping:

```c
mixed *a = ({ 1, 2, 3 });
mixed *b = a;

pop_array(a);
// both a and b are ({ 1, 2 })
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

    Do not remove elements from an array a `foreach` is currently walking.
    A `foreach` visits as many elements as the array held when the loop was
    entered, so removing elements leaves it addressing one that no longer
    exists, and that is reported as an error rather than silently visiting
    fewer elements:

```c
mixed *a = ({ 1, 2, 3, 4 });

foreach (mixed x in a) {
  pop_array(a);          // visits 1, visits 2, then errors
}
```

    Use a `while` loop to drain an array:

```c
while (sizeof(a)) {
  do_something(pop_array(a));
}
```

    Note this differs from a mapping, where deleting entries during a
    `foreach` is tolerated.  A mapping copies its key list when the loop
    starts, so it can afford to; an array loop copies nothing.  Appending
    during a loop is always fine -- see push_array(3).

### SEE ALSO

    push_array(3), shift_array(3), unshift_array(3), undefinedp(3),
    sizeof(3)
