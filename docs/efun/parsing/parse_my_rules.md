---
layout: doc
title: parsing / parse_my_rules
---

# parse_my_rules

### NAME

    parse_my_rules() - query an object's parser rules

### SYNOPSIS

    mixed *parse_my_rules();

### DESCRIPTION

    Returns an array containing all the parser rules currently registered
    for the current object. Each element in the array is a string containing
    the verb and its associated rule pattern.

    This is useful for debugging or when you need to know what commands
    an object currently supports.

### SEE ALSO

    parse_add_rule(3), parse_init(3)
