---
layout: default
title: parsing / parse_add_rule
---

### NAME

    parse_add_rule() - add parsing rules for a verb

### SYNOPSIS

    void parse_add_rule(string verb, string rule);

### DESCRIPTION

    Here "verb" is the command word (e.g. "look", "read" etc), The "rule" is
    the parsing rule to add. Rules are made up from two parts - tokens, and
    prepositions. Tokens are used to match various objects or strings, and
    prepositions are fixed positional words to specify meaning (like "with"
    or "in").

    The MudOS accepts six tokens that I'm aware of:

    OBJ - matches a single object
    OBS - matches one or more objects
    LIV - matches a single, living object
    LVS - matches one or more living objects
    WRD - matches a single word
    STR - matches one or more words

### SEE ALSO

    add_action(3)

