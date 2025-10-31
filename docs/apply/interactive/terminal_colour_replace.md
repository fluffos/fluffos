---
layout: doc
title: interactive / terminal_colour_replace
---
# terminal_colour_replace

### NAME

    terminal_colour_replace - customreplace colour tokens before lookup

### SYNOPSIS

    string terminal_colour_replace(string token) ;

### DESCRIPTION

    This apply is called by the `terminal_colour()` efun to allow custom
    replacement of colour tokens before they are looked up in the provided
    colour mapping.

    **Arguments:**
    - `token`: The colour token string found between %^...%^ markers

    **Return Value:**
    - Return a string to use as the replacement token for mapping lookup
    - Return 0 or non-string to use the original token

    This apply is useful for implementing dynamic colour schemes, aliases,
    or preprocessing of colour tokens. For example, you could implement
    colour token inheritance or transform token names before they're
    looked up in the colour mapping.

### EXAMPLE

    ```c
    string terminal_colour_replace(string token) {
        // Convert uppercase tokens to lowercase
        if (token == "RED") return "red";
        if (token == "BLUE") return "blue";

        // Allow aliases
        if (token == "danger") return "red";
        if (token == "success") return "green";

        // Return 0 to use original token
        return 0;
    }
    ```

### SEE ALSO

    terminal_colour(3)

### NOTE

    This apply is called for each colour token found in the string passed
    to `terminal_colour()`. The token string does not include the %^ markers.
