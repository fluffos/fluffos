---
layout: default
title: contrib / highest
---

### NAME

    highest() - returns the highest value amongst a number of passed values

### SYNOPSIS

    (int | float) highest( int | float, int | float, int | float, ...)

### DESCRIPTION

    Given a number of parameters, find and return the highest value. The parameters may be integers or floats, but they must all be of the same type.

### EXAMPLE

    int result = highest( 1, 2, 5, 4, 2 ); // 5
    float result = highest( 2.2, 5.5, 3.14 ); // 5.5

### SEE ALSO

    min, max, lowest
