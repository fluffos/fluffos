---
layout: doc
title: general / refp
---
# refp

### NAME

    refp() - determine whether or not a given variable is a reference.

### SYNOPSIS

    int refp( mixed arg );

### DESCRIPTION

    Return 1 if 'arg' is a reference, 0 otherwise. A reference is created
    using the 'ref' keyword and points to another variable, allowing
    modifications to affect the original variable.

### EXAMPLES

    void test_function(int ref value) {
        if (refp(value)) {
            write("Parameter was passed by reference\n");
            // Safe to modify the original variable
            value = 100;
        }
    }

    int x = 42;
    test_function(ref x);  // Prints message, x becomes 100

    // refp() returns 0 for non-reference values
    int y = 42;
    refp(y);  // returns 0 (y is not a reference)

### RETURN VALUE

    Returns 1 if the argument is a reference, 0 otherwise.

### SEE ALSO

    nullp(3), typeof(3), stringp(3), intp(3), arrayp(3), mapp(3), objectp(3)
