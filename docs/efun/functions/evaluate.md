---
layout: doc
title: functions / evaluate
---
# evaluate

### NAME

    evaluate() - evaluate a function pointer

### SYNOPSIS

    mixed evaluate(mixed f, ...)

### DESCRIPTION

    If f is a function, f is called with the rest of the arguments.  Other‐
    wise, f is returned.  evaluate(f, ...) is the same as (*f)(...).

### SYNTAX SUGAR

    FluffOS supports simplified syntax for invoking function pointers:

    ```c
    function f = (: add :);
    
    // Traditional syntax
    int result = evaluate(f, 5, 3);
    
    // Simplified syntax (equivalent)
    int result = f(5, 3);
    
    // Legacy syntax (also works)
    int result = (*f)(5, 3);
    ```

    All three forms are functionally equivalent. The simplified `f(args)` syntax
    is recommended for new code as it's cleaner and more intuitive.

### SEE ALSO

    types/function

