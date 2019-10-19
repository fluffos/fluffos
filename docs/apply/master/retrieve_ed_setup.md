---
layout: default
title: master / retrieve_ed_setup
---

### NAME

    retrieve_ed_setup  -  retrieve  a  user's editor setup or configuration
    settings

### SYNOPSIS

    int retrieve_ed_setup( object user );

### DESCRIPTION

    This master apply is called by the ed() efun to retrieve  a  user's  ed
    setup/configuration  settings.   This  function should return the setup
    (contained in an int).

### SEE ALSO

    save_ed_setup(4)

