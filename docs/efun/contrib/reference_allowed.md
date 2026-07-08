---
layout: doc
title: contrib / reference_allowed
---
# reference_allowed

### NAME

    reference_allowed() - Discworld playtester reference check

### SYNOPSIS

    int reference_allowed( object referee, string|object|void referrer );

### DESCRIPTION

    Discworld-mudlib helper: returns whether `referrer` (defaulting to
    this_player(), or looked up by name if a string is given) is allowed
    to hold a reference to `referee`, consulting the mudlib's playtester
    and player handlers. Returns 0 when the handlers are absent, so it is
    only meaningful in a mudlib that provides them.

### SEE ALSO

    this_player(3)
