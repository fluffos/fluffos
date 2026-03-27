---
layout: doc
title: constructs / function
---
# function

### The LPC function (or method)

The LPC function is similar but not identical to that provided by C
(it is most similar to that provided by ANSI C). The syntax is as follows:

```
return_type function_name(arg1_type arg1, arg2_type arg2, ...)
{
    // Variables can now be declared anywhere in the function body,
    // not just at the top (C99-style)
    variable_declarations;
    ...;

    statements;
    ...;

    more_variable_declarations;  // Declarations can be interspersed
    ...;

    return var0;
}
```

Note that var0 must be of return_type.

If a function doesn't need to return a value, then it should be declared
with a return_type of "void". E.g.

```
void function_name(arg1_type arg1, ...)
{
    statements;
    ...;
}
```

Invoke a function as follows:

    function_name(arg1, arg2, arg3, ...);

You may invoke a function in another object as follows:

    object->function_name(arg1, arg2, arg3, ...);

or:

    call_other(object, function_name, arg1, arg2, ...);
