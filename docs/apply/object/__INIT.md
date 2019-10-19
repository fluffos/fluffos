---
layout: default
title: object / __INIT
---

### NAME

    __INIT - obsolete apply

### SYNOPSIS

    __INIT( void );

### DESCRIPTION

    This function used to be called in objects right before create.  global
    variable initialization is now handled by another function that  cannot
    be interfered with, so this is no longer called.

