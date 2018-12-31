---
layout: default
title: master / valid_save_binary
---

### NAME

    valid_save_binary - controls whether or not an object can save its pro‐
    gram

### SYNOPSIS

    int valid_save_binary( string file );

### DESCRIPTION

    When the driver is compiled with  ALWAYS_SAVE_BINARIES,  or  an  object
    uses  If  valid_save_binary returns 1, the save will be allowed, other‐
    wise it will not.
