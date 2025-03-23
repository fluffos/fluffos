---
layout: doc
title: parsing / parse_refresh
---

# parse_refresh

### NAME

    parse_refresh() - refresh an object's parser rules

### SYNOPSIS

    void parse_refresh();

### DESCRIPTION

    Refreshes the parser rules for the current object. This is useful when
    you've made changes to an object's rules and need to ensure they are
    properly registered with the parser.

    The parser caches information about objects (including names and IDs) to
    improve performance. When any of this cached information changes (such
    as when an object's name changes or when adjectives change as a spell
    transforms an object from blue to red), you must call parse_refresh()
    to clear the cache.

    This function is typically called after:

    * Modifying an object's rules
    * Changing an object's name or description
    * Updating an object's adjectives or properties
    * Ensuring the parser has the latest rule configuration

### SEE ALSO

    parse_init(3), parse_add_rule(3)
