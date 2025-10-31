---
layout: doc
title: concepts / references
---
# References

### DESCRIPTION

References in LPC provide a way to pass variables by reference rather than by value, allowing functions to modify the original variable rather than working with a copy. This is particularly useful for functions that need to return multiple values or modify large data structures efficiently.

### CREATING REFERENCES

References in LPC are created and used in two specific contexts:

**1. Function parameter declarations** - Use `ref` as a type modifier:

```lpc
void modify_value(int ref value) {
    value = 100;  // This modifies the original variable
}
```

**2. Function argument expressions** - Use `ref` when calling functions:

```lpc
int x = 42;
modify_value(ref x);  // x is now 100
```

**Important:** You cannot declare local variables as references. The `ref` keyword can only be used in function parameters (as a type modifier) and in function arguments (to create a reference to pass).

### DEREFERENCING

To access the value that a reference parameter holds, use the `deref` keyword. This is typically used within functions that receive reference parameters:

```lpc
void show_value(int ref value) {
    int current = deref value;  // Get the current value
    write("Current value: " + current + "\n");
    value = 200;  // Modify the original
}

int x = 42;
show_value(ref x);  // Prints "Current value: 42", then x becomes 200
```

### CHECKING FOR REFERENCES

Use the `refp()` efun to determine if a function parameter is a reference:

```lpc
void check_param(mixed ref param) {
    if (refp(param)) {
        write("Parameter was passed by reference\n");
        // Safe to modify - will affect original variable
        param = "modified";
    }
}

string value = "original";
check_param(ref value);  // Prints message, value becomes "modified"
```

### PRACTICAL EXAMPLES

**Multiple return values:**
```lpc
void parse_coordinates(string input, int ref x, int ref y) {
    sscanf(input, "%d,%d", x, y);
}

int x, y;
parse_coordinates("10,20", ref x, ref y);
// x is now 10, y is now 20
```

**Modifying arrays efficiently:**
```lpc
void append_to_array(mixed ref *arr, mixed value) {
    arr += ({ value });
}

mixed *my_array = ({ 1, 2, 3 });
append_to_array(ref my_array, 4);
// my_array is now ({ 1, 2, 3, 4 })
```

**Working with mappings:**
```lpc
void increment_counter(mapping ref counters, string key) {
    if (!counters[key]) counters[key] = 0;
    counters[key]++;
}

mapping stats = ([]);
increment_counter(ref stats, "login");
// stats is now ([ "login" : 1 ])
```

### KEYWORDS AND FUNCTIONS

- `ref` - Declare a function parameter as accepting a reference, or pass a variable by reference to a function
- `deref` - Get the value from a reference parameter (can be used anywhere)
- `refp()` - Check if a function parameter is a reference

### NOTES

1. **References can only be used in function contexts** - you cannot declare local variables as references.

2. **Function parameters** must be declared with `ref` type modifier to accept references: `void func(int ref param)`

3. **Function arguments** must use `ref` keyword to pass by reference: `func(ref variable)`

4. **`deref` can be used anywhere** on reference parameters to access the current value.

5. References maintain a connection to the original variable - changes to the reference parameter affect the original.

6. When passing large data structures (arrays, mappings, objects), using references can be more efficient than copying.

7. References are particularly useful for functions that need to modify multiple variables or return multiple values.

8. A reference becomes invalid if the original variable is destroyed or goes out of scope.

### SEE ALSO

refp(3), nullp(3), typeof(3)
