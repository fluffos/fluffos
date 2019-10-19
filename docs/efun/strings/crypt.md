---
layout: default
title: strings / crypt
---

### NAME

    crypt() - encrypt a string

### SYNOPSIS

    string crypt( string str, string seed );

### DESCRIPTION

    Crypt  the string <str> using the first two characters from <seed> as a
    seed. If <seed> is 0, then random seed is used.

    The result has the first two characters as the seed.

