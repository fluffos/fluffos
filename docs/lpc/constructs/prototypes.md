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
return_type function_name(type arg1, type arg2 = default_value);
```

**Rules:**
- Default arguments must appear after all non-default arguments
- Default values must be compile-time constants (numbers, strings, or constant expressions)
- Once a parameter has a default value, all following parameters must also have defaults
- Default arguments work in function prototypes, definitions, and simul_efun declarations

**Examples:**
```c
// Function with default arguments
void greet(string name, string title = "friend") {
    write("Hello, " + title + " " + name + "!\n");
}

// Can be called with different number of arguments
greet("Alice");              // Uses default: "Hello, friend Alice!"
greet("Bob", "Sir");         // Override default: "Hello, Sir Bob!"

// Multiple default arguments
int calculate(int base, int multiplier = 1, int offset = 0) {
    return base * multiplier + offset;
}

calculate(10);               // Returns 10 (multiplier=1, offset=0)
calculate(10, 5);            // Returns 50 (offset=0)
calculate(10, 5, 3);         // Returns 53

// Default arguments with different types
void log_message(string msg, int level = 1, int timestamp = 0) {
    if (timestamp == 0) {
        timestamp = time();
    }
    write(sprintf("[%d] Level %d: %s\n", timestamp, level, msg));
}

log_message("Error occurred");           // Uses defaults
log_message("Warning", 2);               // Custom level
log_message("Info", 3, 1234567890);      // All custom values
```

**Use Cases:**
```c
// API functions with optional parameters
void send_message(object user, string message, int color = 0) {
    if (color) {
        tell_object(user, sprintf("%%^COLOR%%^%s%%^RESET%%^\n", message));
    } else {
        tell_object(user, message + "\n");
    }
}

// Configuration with sensible defaults
void configure_room(string desc, int light_level = 50, int is_indoors = 1) {
    set_short(desc);
    set_light(light_level);
    if (is_indoors) {
        set_property("indoors", 1);
    }
}

// Object creation helpers
object create_monster(string name, int level = 1, int hp = 0) {
    object mob;
    mob = new("/std/monster");
    mob->set_name(name);
    mob->set_level(level);
    mob->set_hp(hp == 0 ? level * 10 : hp);  // Default HP based on level
    return mob;
}
```

**Important Notes:**
- Default argument feature added in FluffOS v2019+
- This is a compile-time feature; the defaults are inserted at compilation
- Default arguments do not work with varargs (`...`) functions
- Default values are evaluated once at compile time, not at each function call
- In function pointers, default arguments from the original function are preserved
