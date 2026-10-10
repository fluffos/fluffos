---
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

A float that the compiler knows is a float is truncated, fractional part
discarded, when it lands in an `int` slot: an assignment, an initializer, an
`op=` such as `x *= 0.85` (the `0.85` becomes `0` before the multiply), or a
`return` from an `int` function. That is deliberate, but it is easy to write
by accident, so the compiler warns:

    int b = 2.75;          // warning: Float value truncated to int ...
    int x = 1000;
    x *= 0.85;             // warning -- x becomes 0, not 850
    int ok = to_int(2.75); // explicit: no warning
    int i = 2.0;           // nothing is lost: no warning

The warning is on by default and `#pragma no_warnings` silences it (see
[diagnostics](../diagnostics)). It only fires when the value is statically a
float; a `mixed` value, a `call_other` result or a mapping/array element has no
static type to check.

The LPC float type is a C `double`: about fifteen (15) significant
decimal digits.
