---
layout: default
title: contrib / test_load
---

### NAME

    test_load - test if a file is loadable

### SYNOPSIS

    int test_load( string filename );

### DESCRIPTION

    Tests if a file is loadable. Will return 1 if a file is loadable,
    otherwise 0. If a file attempting to be loaded contains errors,
    they will be reported, in which case, you may need to wrap the
    function call in a catch statement to retrieve the 0 result.

