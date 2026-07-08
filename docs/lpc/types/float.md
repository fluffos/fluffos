---
layout: doc
title: types / float
---
# float

MudOS LPC now provides support for the floating point type. Declare
variables like this:

    float pi;

In general the same operations are supported for floats as are for integers.
Float literals are written with a decimal point, a trailing dot, or
exponent notation (with or without a fraction), and digits may be
grouped with `_` separators:

    pi = 3.14159265;
    x = 1.;          // 1.0
    big = 2.5e6;     // 2500000.0
    tiny = 2.5e-3;   // 0.0025
    k = 1_000.25;

Note that casts do not convert values — use `to_float()` / `to_int()`
to convert between floats and integers.

The LPC float type is a C `double`: about fifteen (15) significant
decimal digits.
