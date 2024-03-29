---
layout: doc
title: general / stackmachine
---
# stackmachine

THIS IS A DOCUMENT THAT DESCRIBES HOW A VIRTUAL STACK MACHINE HAS BEEN DEFINED, TO EXECUTE COMPILED LPC CODE.

There are two stacks:

1.  The stack of values, used for evaluation, local variables and arguments.
    Note that arguments are treated as local variables. Every element on the
    value stack will have the format `struct svalue`, as defined in `interpret.h`.
    The value stack is stored in an array, with limited size. The first push
    operation will store a value into element 0. Access of arguments and local
    variables are supposed to be fast, by simply indexing in the value stack
    using the frame pointer as offset.

```
    Start of stack -----
                    |
    Frame pointer-> | Argument number 0
                    | Argument number 1
                    | etc.
                    | Local variable number 0
                    | Local variable number 1
                    | etc.
                    | Temporary stack values
                    | etc
                    v
```

2.  The control stack that contains return addresses, frame pointer etc.

## CALLING LOCAL FUNCTIONS.

All arguments are evaluated and pushed to the value stack. The last argument
is the last pushed. It is important that the called function gets exactly as
many arguments as it wants. The number of arguments will be stored in the
control stack, so that the return instruction not needs to know it
explicitly.

Instruction format:

```
    b0 b1 b2 b3

    b0 = F_FUNCTION.
    b1, b2 = The number of the function to be called.
    b3 = Number of arguments sent.

```

The `F_FUNCTION` instruction will also initiate the frame pointer to point
to the first argument.

The number of arguments are stored in the 'struct function' which is found
using the number of the function and indexing in `ob->prog->functions[]`;
The number of arguments will be adjusted to fit the called function.
This is done by either pushing zeroes, or poping excessive
arguments. `F_FUNCTION` will also initiate local variables, by pusing a 0
for each of them.

The called function must ensure that exactly one value remains on the
stack when returning. The caller is responsible of deallocating the
returned value.

When a function returns, it will use the instruction F_RETURN, which will
deallocate all arguments and local variables, and only let the top of stack
entry remain. The number of arguments and local variables are stored in the
control stack, so that the evaluator knows hoh much to deallocate.

If flag `extern_call` is set, then the evaluator should return. Otherwise,
the evaluator will continue to execute the instruction at the returned
address.

```
    Format:

    b0

    b0 = F_RETURN.
```

## CALLING PREDEFINED FUNCTIONS.

Arguments are pushed to the stack. A value is always returned (on the stack).

Instruction format:

```
    b1

    b1 = The F\_ code of the called function.
```

If a variable number of arguments are allowed, then an extra byte will
follow the instruction, that states number of actual arguments.

The execution unit will parse number of arguments immediately, regardless
of which instruction it is when it is stated that a variable number of
arguments are allowed. It will also check soem of the types of the
arguments immediately, if it is possible. But never more than the types of
the first two arguments.

## F_SSCANF

The function sscanf is special, in that arguments are passed by reference.
This is done with a new type, T_LVALUE. The compiler will recognize
sscanf() as a special function, pass the value of the two first arguments
as normal rvalues and pass the rest as lvalues. The total number of arguments
is given as a one byte code supplied to the F_SSCANF instruction.

## F_CALL_OTHER

This command takes one argument, a byte which gives the number of arguments.

```
    b1, b2

    b1 = F_CALL_OTHER, b2 = number of arguments.
```

## F_AGGREGATE

This command takes one argument, the size of the array. The elements of
the array are picked from the top of stack.

```
    b1, b2, b3

    b1 = F_AGGREGATE, (b2,b3) = Size of the array (max 0xffff).
```

## F_CATCH

The compiler constructs a call to F_CATCH before the code to evaluate the
argument of F_CATCH. After the code, a call to F_RETURN is made. Thus,
it will look like a function call.

F_CATCH will when executed do setjmp() and call eval_instruction()
recursively. That means that a new frame has to be set up.

F_THROW will do a longjmp().

format:

```
    F_THROW, b1, b2, (instructions...), F_RETURN

    Where b1,b2 is the address of the instruction after the return instruction.
```

## F_RETURN

Will deallocate the current frame, and restore the previous. If the flag
extern_call is set, then a return from eval_instruction() will be done.
