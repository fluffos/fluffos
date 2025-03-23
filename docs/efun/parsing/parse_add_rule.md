---
layout: doc
title: parsing / parse_add_rule
---

# parse_add_rule

### NAME

    parse_add_rule() - add a rule for a verb

### SYNOPSIS

    void parse_add_rule(string verb, string rule);

### DESCRIPTION

    Adds a rule pattern for the specified verb. The rule parameter defines
    how the command should be parsed, including what objects and prepositions
    are expected.

    The rule pattern follows a specific syntax:

    * Direct objects: 'OBJ'
    * Indirect objects: 'OBJ2' (or second 'OBJ' in rule)
    * Prepositions: 'PREP'
    * Words: 'WRD' (matches a single word)
    * Strings: 'STR' (matches one or more words)
    * Living objects: 'LIV'
    * Multiple objects: 'OBS'
    * Multiple living objects: 'LVS'

    Example:

    ```c
    parse_add_rule("give", "OBJ to OBJ");  // Second OBJ is indirect object
    parse_add_rule("look", "at OBJ");
    parse_add_rule("put", "OBJ in OBJ");   // Second OBJ is indirect object
    ```

    Additional examples:

    ```c
    // WRD matches a single word
    parse_add_rule("say", "WRD");          // matches: "say hello"
    parse_add_rule("emote", "WRD");        // matches: "emote smiles"

    // STR matches one or more words
    parse_add_rule("say", "STR");          // matches: "say hello there"
    parse_add_rule("emote", "STR");        // matches: "emote smiles warmly"
    ```

### SEE ALSO

    parse_init(3), parse_remove(3)
