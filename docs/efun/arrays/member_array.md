---
layout: default
title: arrays / member_array
---

### NAME

    member_array()  -  returns  index of an occurence of a given item in an
    array or string

### SYNOPSIS

    int member_array( mixed item, mixed * | string arr, void | int anker, void | int flag );

### DESCRIPTION

    Returns the index of the first occurence of 'item' in array/string 'arr',
    or the first occurence starting at or after 'anker'.  If the item is not
    found, then -1 is returned.

    If bit 1 of 'flag' is set, this efun returns the index of the last
    occurance of 'item' ending before or at 'anker'.
