---
layout: default
title: stdlib / percent_of
---

### NAME

    percent_of() - returns a percent as either int or float, depending upon
    arguments

### SYNOPSIS

    int|float percent_of( int|float numerator, int|float denomintor );

### DESCRIPTION

    This function will return the percentage that is calculated by numerator
    divided by denominator and then divided by 100.

    If either the value or the total is a float, then a float will be returned,
    otherwise int will be returned.

### EXAMPLE

```c
percent_of( 25, 50 ) //Result 12
percent_of( 25.0, 50 ) //Result: 12.500000
```

### SEE ALSO

    percent_of
