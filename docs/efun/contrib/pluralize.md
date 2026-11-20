---
title: contrib / pluralize.pre
---
# pluralize

### NAME

    pluralize - return plural form

### SYNOPSIS

    string pluralize(string);

### DESCRIPTION

    for english only!

    returns plural form:
    'a red house' -> 'red houses'
    'a sack of rice' -> 'sacks of rice'

Only the last word before any " of " is pluralized. It is checked against a
table of exceptions (child -> children, sheep -> sheep, ...) before the
general suffix rules apply. The mudlib can add to or override that table with
[update_pluralize()](update_pluralize).

A last word of one letter is treated as a letter or a label and takes 's,
whatever its case: "rune f" -> "rune f's", "vitamin C" -> "vitamin C's".
An entry in the exception table for that letter takes precedence.

### SEE ALSO

    query_num(3), update_pluralize(3)
