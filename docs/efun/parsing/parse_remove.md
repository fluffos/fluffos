---
layout: doc
title: parsing / parse_remove
---

# parse_remove

### NAME

    parse_remove() - remove all rules for a verb

### SYNOPSIS

    void parse_remove(string verb);

### DESCRIPTION

    Removes all rules associated with the specified verb from the parser.
    This effectively disables the verb and all its synonyms until new rules
    are added using parse_add_rule().

    This is useful when you need to completely reset a verb's configuration
    or when you want to temporarily disable certain commands.

### SEE ALSO

    parse_add_rule(3), parse_init(3)
