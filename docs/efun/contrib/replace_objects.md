---
title: contrib / replace_objects
---
# replace_objects

### NAME

    replace_objects() - recursively render objects in a value as strings

### SYNOPSIS

    mixed replace_objects( mixed value );

### DESCRIPTION

    Walks `value` (an object, array, class, or mapping, to any depth) and
    returns a copy in which every object is replaced by a descriptive
    string — its object name plus, via the master apply object_name(),
    a readable label, or "(destructed)" for a destructed object. Arrays,
    classes and mappings keep their structure; other values pass through
    unchanged. Chiefly a debugging / dump helper.

### SEE ALSO

    sprintf(3)
