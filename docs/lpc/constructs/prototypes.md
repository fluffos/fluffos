---
layout: doc
title: constructs / prototypes
---
# prototypes

### The function prototype

The LPC function prototype is very similar to that of ANSI C. The
function prototype allows for better type checking and can serve as
a kind of 'forward' declaration.

    return_type function_name(arg1_type arg1, arg2_type arg2, ...);

Also note that the arguments need not have names:

    return_type function_name(arg1_type, arg2_type, ...);

### Default Arguments

FluffOS supports default argument values in function prototypes and definitions,
allowing you to specify default values for parameters that can be omitted when
calling the function.

**Syntax:**
```c
return_type function_name(type arg1, type arg2: (: default_value :));
```

Default arguments use FluffOS's functional syntax `: (: expression :)` where the
expression is evaluated when the argument is not provided.

**Rules:**
- Default arguments must appear after all non-default arguments
- Default values are specified using functional syntax: `: (: expression :)`
- Once a parameter has a default value, all following parameters must also have defaults
- Default arguments work in function prototypes, definitions, and simul_efun declarations
- Can be combined with varargs functions

**Examples:**
```c
// Function with default arguments
void greet(string name, string title: (: "friend" :)) {
    write("Hello, " + title + " " + name + "!\n");
}

// Can be called with different number of arguments
greet("Alice");              // Uses default: "Hello, friend Alice!"
greet("Bob", "Sir");         // Override default: "Hello, Sir Bob!"

// Multiple default arguments
int calculate(int base, int multiplier: (: 1 :), int offset: (: 0 :)) {
    return base * multiplier + offset;
}

calculate(10);               // Returns 10 (multiplier=1, offset=0)
calculate(10, 5);            // Returns 50 (offset=0)
calculate(10, 5, 3);         // Returns 53

// Default arguments with different types
void test4(int a, string b: (: "str" :), int c: (: -1 :)) {
    write(sprintf("%d, %s, %d\n", a, b, c));
}

test4(1);                    // Uses defaults: "1, str, -1"
test4(2, "aaa");             // Override b: "2, aaa, -1"
test4(3, "bbb", 1);          // All custom values: "3, bbb, 1"
```

**Use Cases:**
```c
// Default value evaluated in caller's context
object test5(object a: (: this_object() :)) {
    return a;
}

// Works with varargs
varargs string test7(string a, string b: (: "bbb" :), string c: (: "ccc" :)) {
    return a + b + c;
}

test7();                     // Returns "0bbbccc"
test7("aaa");                // Returns "aaabbbccc"
test7("aaa", "xxx");         // Returns "aaaxxxccc"
test7("aaa", "xxx", "yyy");  // Returns "aaaxxxyyy"

// Object creation helpers
object create_monster(string name, int level: (: 1 :), int hp: (: 0 :)) {
    object mob;
    mob = new("/std/monster");
    mob->set_name(name);
    mob->set_level(level);
    mob->set_hp(hp == 0 ? level * 10 : hp);
    return mob;
}
```

**Important Notes:**
- Default argument feature added in FluffOS v2019+
- Default values are evaluated at call time in the **caller's context**, not at compile time
- The expression in the functional can reference the caller's variables and functions
- Default arguments work with varargs functions (unlike some other LPC dialects)
- Works with direct calls, apply (call_other), and inherited functions
- The functional syntax `: (: expr :)` allows complex expressions, not just constants
