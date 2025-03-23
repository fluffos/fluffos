---
layout: doc
title: parsing / query_verb
---

# query_verb

### NAME

    query_verb() - get the current command being executed

### SYNOPSIS

    string query_verb();

### DESCRIPTION

    Returns the name of the command currently being executed, or 0 if not
    executing from a command. This is useful when you need to know what
    command triggered the current function call.

    This is particularly helpful in functions that handle multiple commands
    and need to determine which command was used to call them.

### SEE ALSO

    parse_command(3), parse_init(3)
