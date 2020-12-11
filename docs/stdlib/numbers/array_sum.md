---
layout: default
title: stdlib / array_sum
---

### NAME

    array_sum() - return the sum of all of the numeric elements of the passed array

### SYNOPSIS

    int|float sum( mixed* );

### DESCRIPTION

    This function will aggregate the values of all elements of the passed
    array into a single return value. The array may be any combination of
    int or float and the appropriate type will be returned based on the
    final total.

### EXAMPLE

```c
array_sum( ({ 1, 2, 3, 4, 5, 6 }) ); // Result: 21
array_sum( ({ 1, 2, 3.5, 4, 5, 6 }) ); // Result: 21.500000
```

### SEE ALSO

    sum
