---
layout: default
title: object / is_living
---

### NAME

   is_living() - determine whether an object is alive or not

### SYNOPSIS

   int is_living();

### DESCRIPTION

    This apply is used by the parser to determine whether an object is alive
    or not, that is to say an NPC or player as opposed to a table. It should
    return an int 1 for "yes, I'm alive" and 0 for "no, I'm not alive."

    This response is used to check whether the object is allowed to match a
    "LIV" rule (see below) or not. The response is cached and so should it
    change parse_refresh() will need to be called before the will notice.

### SEE ALSO

   inventory_visible(4), inventory_accessible(4)
