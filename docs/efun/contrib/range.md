---
layout: default
title: contrib / range
---

### NAME

    range

### SYNOPSIS

    (int | float) range( int | float lower_bound, int | float upper_bound, int | float value_to_test )

### DESCRIPTION

    Range will test value_to_test against lower_bound and upper_bound and return value_to_test if it falls within the provided range.

    If value_to_test falls below lower_bound, then lower_bound is returned.
    If value_to_test falls above upper_bound, then upper_bound is returned.

    Acceptable types are integer and float, however, all of the parameters must be of the same type.

### EXAMPLE

    int result = range(1, 5, -4); // 1
    float result = range(5.0, 100.0, 55.0); // 55.0
