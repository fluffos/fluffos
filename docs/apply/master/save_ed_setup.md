---
layout: default
title: master / save_ed_setup
---

### NAME

    save_ed_setup - save a user's editor setup or configuration settings

### SYNOPSIS

    int save_ed_setup( object user, int config );

### DESCRIPTION

    This  master  apply is called by the ed() efun to save a user's ed set‐
    up/configuration settings (contained in an int).  This function  should
    return an int for success (1 or TRUE)/failure (0 or FALSE).

### SEE ALSO

    retrieve_ed_setup(4)

