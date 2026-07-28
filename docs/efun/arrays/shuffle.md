---
title: arrays / shuffle
---
# shuffle

### NAME

    shuffle() - rearrange the elements of an array in random order

### SYNOPSIS

    mixed *shuffle(mixed *arr);

### DESCRIPTION

    Rearranges the elements of `arr` into a random order.

    The shuffle happens in place: `arr` itself is reordered, not a copy
    of it.  Every variable holding a reference to that array sees the
    new order after the call.

### RETURN VALUE

    Returns `arr` itself -- the same array that was passed in, after
    shuffling -- not a shuffled copy.  The return value is a convenience
    for use in expressions; ignoring it changes nothing, since the
    argument array has already been reordered by the time the call
    returns.

    An array with fewer than two elements is returned unchanged.

### EXAMPLE

```c
mixed *a = ({ 1, 2, 3, 4, 5 });
mixed *b = shuffle(a);
// a itself has been reordered; b is that same array, not a copy.

// To keep the original order intact, shuffle a copy instead:
mixed *c = shuffle(a[0..]);
// the range expression builds a new array, so a is untouched
```

### SEE ALSO

    element_of(3), sort_array(3), random(3)
