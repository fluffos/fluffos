---
layout: default
title: stdlib / sum
---

### NAME

    sum() - return the sum of all of the passed int or float parameters

### SYNOPSIS

    int|float sum( int|float... );

### DESCRIPTION

    This function will aggregate the values of all arguments into a single
    return value. The arguments may be any combination of int or float and
    the appropriate type will be returned based on the final total.

### EXAMPLE

```c
sum( 1, 2, 3, 4, 5, 6 ); // Result: 21
sum( 1, 2, 3.5, 4, 5, 6 ); // Result: 21.500000
```

### SEE ALSO

    array_sum
