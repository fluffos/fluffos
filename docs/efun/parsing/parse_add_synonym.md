---
layout: doc
title: parsing / parse_add_synonym
---

# parse_add_synonym

### NAME

    parse_add_synonym() - add a synonym for a verb

### SYNOPSIS

    void parse_add_synonym(string new_verb, string old_verb);

### DESCRIPTION

    Adds a synonym for an existing verb. The new_verb will be treated exactly
    like the old_verb, allowing players to use either form of the command.

    Example:

    ```c
    // "examine" works exactly like "look"
    parse_add_synonym("examine", "look");
    ```

### SEE ALSO

    parse_add_rule(3), parse_remove(3)
