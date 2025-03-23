---
layout: doc
title: parsing / parse_dump
---

# parse_dump

### NAME

    parse_dump()

### SYNOPSIS

    string parse_dump();

### DESCRIPTION

    Returns a string containing a dump of all verbs and their rules currently
    registered with the parser. This is primarily useful for debugging purposes
    to see what verbs are available and how they are configured.

    The output includes information about each verb, its synonyms, and the
    rules associated with it. This can be helpful when trying to understand
    why certain commands aren't working as expected or when debugging parser
    configuration issues.

### SEE ALSO

    parse_init(3), parse_add_rule(3)
