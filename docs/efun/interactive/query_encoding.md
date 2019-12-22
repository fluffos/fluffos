---
layout: default
title: interactive / query_encoding
---

### SYNOPSIS

    string query_encoding();

### DESCRIPTION

    get the input/ouput encoding for current player.

    Note: the name you get are ICU internal names, which will be the same returned by set_encoding(), but may or
          maynot be the one you passed in, but they refer to the same encoding.

### SEE ALSO

    set_encoding

