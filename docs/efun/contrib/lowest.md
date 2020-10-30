---
layout: default
title: contrib / lowest
---

### NAME

    lowest() - returns the lowest value amongst a number of passed values

### SYNOPSIS

    (int | float) lowest( int | float, int | float, int | float, ...)

### DESCRIPTION

    Given a number of parameters, find and return the lowest value. The parameters may be integers or floats, but they must all be of the same type.

### EXAMPLE

    int result = lowest( 1, 2, 5, 4, 2 ); // 1
    float result = lowest( 2.2, 5.5, 3.14 ); // 2.2

### SEE ALSO

    min, max, highest
