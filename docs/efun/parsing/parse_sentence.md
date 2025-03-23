---
layout: doc
title: parsing / parse_sentence
---

# parse_sentence

### NAME

    parse_sentence() - parse the command contained in the given string

### SYNOPSIS

    mixed parse_sentence(string command[, int debug_level[, object *oblist[,
    mapping can]]]);

### DESCRIPTION

    This efun calls the driver parser and tells it to parse and execute the
    command contained in the given string. The efun may return an integer
    error code, or a string error message. If a string message is returned,
    it should be displayed to the player.

    The optional parameters are:

    * debug_level: If provided, enables parser debugging output. Must be compiled
                  with -DDEBUG or -DPARSE_DEBUG.
                  Level 0: No debug output
                  Level 1: Basic debug output
                  Level 2+: Verbose debug output
    * oblist: If provided, specifies which objects to search for commands in. If
              not provided, uses the default environment.
    * can: If provided, a mapping of string nicknames to objects. These nicknames
           will be resolved to their corresponding objects during command parsing.

    The return values are:

    * 1: command has been processed ok
    * 0: no matching command was found, no processing done
    * -1: A matching command was found but none of its rules were matched
    * -2: A rule made sense but all "can" or "direct" applies returned 0
    * string: An error message that should be displayed to the player

### CAN MAPPING

    The can mapping is used to provide nickname resolution during command parsing.
    It maps string nicknames to their corresponding objects. When the parser
    encounters a word that matches a nickname, it will substitute the corresponding
    object.

    Example:

    ```c
    mapping can = ([
      "me" : this_player(),
      "here" : environment(this_player()),
      "my sword" : this_player()->query_weapon(),
      "the door" : environment(this_player())->query_door("north")
    ]);
    ```

    With this mapping, commands like:

    * "look at me" will resolve "me" to the player object
    * "get sword from here" will resolve "here" to the current room
    * "open the door" will resolve "the door" to the specific door object

    The mapping values must be objects - other types will be ignored by the parser.
    This is useful for making commands more natural and allowing players to use
    common references instead of having to use exact object names.

### SEE ALSO

    add_action(3), parse_command(3)
