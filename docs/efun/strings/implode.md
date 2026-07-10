---
title: strings / implode
---
# implode

### NAME

    implode() - concatenate strings

### SYNOPSIS

    string implode( mixed *arr, string del );
    mixed implode( mixed *arr, function f, void | mixed extra );

### DESCRIPTION

    In the first form, concatenate all strings found in array 'arr', with
    the  string  'del' between each element. Only strings are used from the
    array; elements that are not strings are ignored.

    In the second form, when the second argument is a function, implode()
    instead  reduces  the  array: it combines the elements left to right by
    repeatedly  calling  'f'  with the running result and the next element
    (a fold), and returns whatever the final call returns. The optional
    'extra' argument, when present, is used as the starting value.

### RETURN VALUE

    The joined string for the delimiter form, or the accumulated value
    (any type) for the function form.

### SEE ALSO

    explode(3), sprintf(3), filter(3), map(3)

