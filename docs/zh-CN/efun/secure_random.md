---
layout: default
title: numbers / secure_random
---

### NAME

    secure_random() - return a pseudo-random number, this should be unpredictable, but maybe slightly slow.

### SYNOPSIS

    int secure_random( int n );

### DESCRIPTION

    Return a cryptographically secure random number from the range [0 .. (n -1)] (inclusive).

    On Linux & OSX, this function explicitly use randomness from /dev/urandom, on windows it is implementation
     defined.
