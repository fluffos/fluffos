---
layout: doc
title: constructs / function
---
# function

### The LPC function (or method)

The LPC function is similar but not identical to that provided by C
(it is most similar to that provided by ANSI C). The syntax is as follows:

```c
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

```c
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

### First-Class Functions

Functions can be treated as first-class values and stored in variables. You can
use function variables to implement callbacks, strategy patterns, and functional
programming techniques.

**Storing functions:**

```c
function f = add;  // Store function reference
int result = f(5, 3);  // Call it later
```

**Passing functions as arguments:**

```c
void apply_operation(int a, int b, function op) {
    int result = op(a, b);
    printf("Result: %d\n", result);
}

int add(int x, int y) { return x + y; }
int multiply(int x, int y) { return x * y; }

void create() {
    apply_operation(5, 3, add);       // Result: 8
    apply_operation(5, 3, multiply);  // Result: 15
}
```

**Returning functions:**

```c
function get_comparator(string type) {
    if (type == "ascending") return (: $1 - $2 :);
    if (type == "descending") return (: $2 - $1 :);
    return 0;
}

void sort_by_type(int *arr, string type) {
    function cmp = get_comparator(type);
    return sort_array(arr, cmp);
}
```

**Storing in data structures:**

```c
// Array of functions
function *operations = ({ add, subtract, multiply, divide });
int result = operations[0](10, 5);

// Mapping of functions
mapping commands = ([
    "attack": (: do_attack :),
    "defend": (: do_defend :),
    "heal": (: do_heal :),
]);

void execute_command(string cmd) {
    function handler = commands[cmd];
    if (handler) handler();
}
```

See also: `types/function` for more details on function pointers and syntax.
