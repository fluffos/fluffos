---
layout: doc
title: types / function
---
# function

## General Concept

FluffOS has a variable type named `function`. Variables of this type may be
used to point to a wide variety of functions. You are probably already familiar
with the idea of passing a function to certain efuns. Take, for example, the
filter efun. It takes an array, and returns an array containing the elements
for which a certain function returns non-zero. Traditionally, this was done by
passing an object and a function name. However, it can also be done by passing
an expression of type 'function' which merely contains information about a
function, which can be evaluated later.

Function pointers can be created and assigned to variables:

```c
    function f = (: local_func :);
    // Or using the simplified syntax:
    function f = local_func;
```

Passed to other routines or efuns, just like normal values:

```c
    foo(f);
    map_array( ({ 1, 2 }), f);
```

Or evaluated at a later time:

```c
    x = evaluate(f, "hi");
    // Or using the simplified invocation syntax:
    x = f("hi");
```

When the last line is run, the function that f points to is called, and "hi" is
passed to it. This will create the same effect as if you had done:

```c
    x = local_func("hi");
```

The advantage of using a function pointer is that if you later want to use a
different function, you can just change the value of the variable.

Note that if evaluate() is passed a value that is not a function, it just
returns the value. So you can do something like:

```c
    void set_short(mixed x) { short = x; }
    mixed query_short() { return evaluate(short); }
```

This way, simple objects can simply do: `set_short("Whatever")`, while objects
that want their shorts to change can do: `set_short( (: short_func :) )`;

## Syntax Sugar (Simplified Syntax)

FluffOS supports a simplified syntax for working with function pointers that makes
code more readable and closer to other modern languages:

### Creating Function Pointers

Instead of wrapping function names in `(: :)`, you can use bare function names:

```c
// Traditional syntax
function f1 = (: add :);
function f2 = (: abs :);

// Simplified syntax (equivalent)
function f1 = add;
function f2 = abs;
```

This works for:

- Local functions in the same object
- Simul efuns
- Built-in efuns

### Invoking Function Pointers

Instead of using `evaluate()`, you can call function variables directly:

```c
function f = add;

// Traditional syntax
int result = evaluate(f, 5, 3);

// Simplified syntax (equivalent)
int result = f(5, 3);
```

### Examples

**Storing functions in data structures:**

```c
// Array of functions
function *operations = ({ add, subtract, multiply, divide });
int result = operations[0](10, 5);  // Calls add(10, 5)

// Mapping of functions
mapping ops = ([
  "add": add,
  "multiply": multiply,
]);
int sum = ops["add"](3, 4);  // Returns 7
```

**Passing functions without wrappers:**

```c
int *numbers = ({ 1, 2, 3, 4, 5 });
int *doubled = map(numbers, (: $(times_two)($1) :));
```

**Returning functions:**

```c
function get_operation(string op) {
  if (op == "add") return add;
  if (op == "multiply") return multiply;
  return 0;
}

function f = get_operation("add");
int result = f(3, 4);  // Returns 7
```

**Backward Compatibility:**
The simplified syntax is fully compatible with the traditional `(: :)` syntax.
You can mix both styles freely:

```c
function f1 = add;              // new style
function f2 = (: multiply :);   // traditional style

int x = f1(3, 4);              // new invocation
int y = evaluate(f2, 3, 4);    // traditional invocation

// Both work with both invocation styles
int a = evaluate(f1, 3, 4);    // works
int b = f2(3, 4);              // works
```

## Available kinds of function pointers

The simplest function pointers are the ones shown above. These simply point to
a local function in the same object, and are made using `(: function_name :)`.
Arguments can also be included; for example:

```c
string foo(string a, string b) {
    return "(" + a "," + b + ")";
}

void create() {
    function f = (: foo, "left" :);

    printf( "%s %s\n", evaluate(f), evaluate(f, "right") );

}
```

Will print:

```
    (left,0) (left,right)
```

The second kind is the efun pointer, which is just `(: efun_name :)`. This is
very similar to the local function pointer. For example, the `objects()` efun
takes a optional function, and returns all objects for which the function is
true, so:

```c
    objects( (: clonep :) )
```

will return an array of all the objects in the game which are clones.
Arguments can also be used:

```c
void create() {
    int i;
    function f = (: write, "Hello, world!\n" :);

    for (i=0; i<3; i++) { evaluate(f); }
}
```

Will print:

```
    Hello, world!
    Hello, world!
    Hello, world!
```

Note that simul_efuns work exactly like efuns with respect to function
pointers.

The third type is the call_other function pointer, which is similar to the type
of function pointer MudOS used to support. The form is `(: object, function :)`.
If arguments are to be used, the should be added to an array along with
the function name. Here are some examples:

```c
void create() {
    string *ret;
    function f = (: this_player(), "query" :);

    ret = map( ({ "name", "short", "long" }), f );
    write(implode(ret, "\n"));
}
```

This would print the results of `this_player()->query("name")`,
`this_player()->query("short")`, and `this_player()->query("long")`.
To make a function pointer that calls `query("short")` directly, use:

```c
    f = (: this_player(), ({ "query", "short" }) :)
```

For reference, here are some other ways of doing the same thing:

```c
// a efun pointer using the call_other efun
f = (: call_other, this_player(), "query", "short" :);
// an expression functional
f = (: this_player()->query("short") :);
```

The fourth type is the expression function pointer. It is made using
`(: expression :)`. Within an expression function pointer, the arguments
to it can be referred to as $1, $2, \$3 ..., for example:

```c
    evaluate( (: $1 + $2 :), 3, 4) // returns 7.
```

This can be very useful for using sort_array, for example:

```c
    top_ten = sort_array( player_list, (:
        $2->query_level() - $1->query_level()
    :) )[0..9];
```

The fifth type is an anonymous function:

```c
void create() {
    function f = function(int x) {
        int y;

        switch(x) {
            case 1: y = 3;break;
            case 2: y = 5;
        }
        return y - 2;
    };

    printf("%i %i %i\n", (*f)(1), (*f)(2), (*f)(3));
}
```

would print:

```
    1 3 -2
```

Note that `(*f)(...)` is the same as `evaluate(f, ...)` and is retained for
backwards compatibility. Anything that is legal in a normal function is
legal in an anonymous function.

## When are things evaluated?

The rule is that arguments included in the creation of efun, local function,
and simul_efun function pointers are evaluated when the function pointer is
made. For expression and functional function pointers, nothing is evaluated
until the function pointer is actually used:

```c
    // When it is _evaluated_, it will destruct whoever "this_player()" was
    // when it was _made_
    (: destruct, this_player() :)

    // destructs whoever is "this_player()" when the function is _evaluated_
    (: destruct(this_player()) :)
```

For this reason, it is illegal to use a local variable in an expression
pointer, since the local variable may no longer exist when the function pointer
is evaluated. However, there is a way around it:

```c
    (: destruct( $(this_player) ) :) // Same as the first example above
```

`$(whatever)` means **evaluate whatever, and hold its value, inserting it
when the function is evaluated**. It also can be used to make things more
efficient:

```c
    map_array(listeners, (:
        tell_object($1, $(this_player()->query_name()) + " bows.\n")
    :) );
```

only does one call_other, instead of one for every message. The string addition
could also be done before hand:

```c
    map_array(listeners, (:
        tell_object($1, $(this_player()->query_name() + " bows.\n"))
    :) );
```

Notice, in this case we could also do:

```c
    map_array(listeners, (:
        tell_object, this_player()->query_name() + " bows.\n"
    :) );
```
