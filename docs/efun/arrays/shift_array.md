---
title: arrays / shift_array
---
# shift_array

### NAME

    shift_array() - remove and return the first element of an array variable

### SYNOPSIS

    mixed shift_array(mixed *arr);

### DESCRIPTION

    Removes the first element of the array held by `arr`, shifting the
    remaining elements down one position, and returns the removed element.

    The argument is a *variable*, not a value: `shift_array()` writes the
    shortened array back into it.  No `ref` keyword is needed at the call
    site -- the driver forms the reference itself.

    Anything assignable holding an array may be the target: a local, a
    global, an element of another array, a mapping value, a `ref`
    parameter, or a `foreach` loop variable declared `ref`.  A function
    call result or a range (`arr[0..2]`) is not assignable and is
    rejected at compile time.

    The array itself is changed, not replaced, so everything holding that
    array sees the element go:

```c
mixed *a = ({ 1, 2, 3 });
mixed *b = a;

shift_array(a);
// both a and b are ({ 2, 3 })
```

    Shifting an empty array is not an error.  It is a no-op that returns
    undefined, the way an out-of-range slice_array() clamps rather than
    failing.

    Do not shift an array a `foreach` is currently walking -- see
    pop_array(3).

### RETURN VALUE

    Returns the element that was removed.

    On an empty array, returns undefined and leaves the variable alone.
    Because an array can legitimately contain 0, test with undefinedp()
    rather than comparing against 0 when the distinction matters.

    An element holding an object that has since been destructed comes back
    as undefined, matching what indexing that element would have yielded.

### EXAMPLE

```c
mixed *queue = ({ "first", "second", "third" });

while (sizeof(queue)) {
  write(shift_array(queue) + "\n");
}
// writes first, second, third -- arrival order
```

    A work queue that grows while it is drained:

```c
mixed *pending = ({ "/room/start" });

while (sizeof(pending)) {
  string path = shift_array(pending);

  foreach (string next in find_exits(path)) {
    if (member_array(next, seen) == -1) {
      push_array(pending, next);
    }
  }
}
```

### SEE ALSO

    unshift_array(3), pop_array(3), push_array(3), undefinedp(3),
    sizeof(3)
