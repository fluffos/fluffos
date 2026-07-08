---
layout: doc
title: contrib / query_multiple_short
---
# query_multiple_short

### NAME

    query_multiple_short() - combine several objects into one short description

### SYNOPSIS

    string query_multiple_short( mixed *items, int|string|void type, int|void no_dollars, int|void quiet, int|void dark );

### DESCRIPTION

    Formats an array of items (objects, or their short strings) into a
    single natural-language list — grouping identical shorts and counting
    them, e.g. "two swords, a shield and some coins".

    The optional arguments tune the output: `type` selects which short
    (a named category) to query from the objects; `no_dollars` suppresses
    the `$`-markup used for colour/where clauses; `quiet` omits items
    flagged as quiet; and `dark` produces the description as seen in the
    dark. This is a Discworld-mudlib inventory helper.

### SEE ALSO

    query_multiple_short(3)
