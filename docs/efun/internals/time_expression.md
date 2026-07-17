---
title: internals / time_expression
---
# time_expression

### NAME

    time_expression - return the amount of real time that an expression
        or block took to evaluate

### SYNOPSIS

    int time_expression( expr );
    int time_expression { statements }

### DESCRIPTION

    time_expression is a language construct (a compiler keyword, like
    catch), not an ordinary efun.  It accepts two body styles, sharing the
    same grammar as catch:

    - Parenthesized expression form: `time_expression(expr)` evaluates
      <expr>.
    - Block form: `time_expression { statements }` executes the statement
      block.

    In both forms, the amount of real (wall-clock) time that passed during
    the evaluation of the body, in microseconds, is returned as an int.
    The value of the body expression itself is discarded.  The precision
    of the value is not necessarily 1 microsecond; in fact, it probably is
    much less precise.

    It is a compile-time error to `break` or `continue` out of a
    time_expression block.

### EXAMPLES

    ```c
    int usec = time_expression( users() );

    int usec = time_expression {
        for (int i = 0; i < 1000; i++)
            do_something();
    };
    ```

### SEE ALSO

    rusage(3), function_profile(3), time(3), catch(3)
