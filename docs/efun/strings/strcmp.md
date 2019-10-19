---
layout: default
title: strings / strcmp
---

### NAME

    strcmp() - determines the lexical relationship between two strings.

### SYNOPSIS

    int strcmp( string one, string two );

### DESCRIPTION

    This  implementatin  of  strcmp()  is  identical  to the one found in C
    libraries.  If string one lexically precedes string two, then  strcmp()
    returns  a number less than 0.  If the two strings have the same value,
    strcmp() returns 0.  If string two lexically precedes string one,  then
    strcmp()  returns  a number greater than 0.  This efunction is particu‐
    larly useful in the compare functions needed by sort_array(3).

### SEE ALSO

    sort_array(3)

