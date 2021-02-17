---
layout: default
title: stdlib / percent
---

### NAME

    percent() - returns a percent as either int or float, depending upon
    arguments

### SYNOPSIS

    int|float percent( int|float value, int|float total );

### DESCRIPTION

    This function will return the percent that value represents of the total.
    If either the value or the total is a float, then a float will be returned,
    otherwise int will be returned.

### EXAMPLE

```c
percent( 1, 5 ) //Result 20
percent( 2.5, 50 ) //Result: 5.000000
```

### SEE ALSO

    percent_of
