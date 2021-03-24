---
layout: default
title: stdlib / base64decode
---

### NAME

    base64decode - decodes a string encoded in base64 into a human-readable format

### SYNOPSIS

    string base64decode( string source ) ;

### ARGUMENTS

    source - the source base64-encoded string to decode

### DESCRIPTION

    Decodes and returns a base64-encoded string into human-readable format.

### EXAMPLES

    printf( "%s", base64decode( "Rmx1ZmZPUyBpcyBncmVhdCEhIDop" ) ) ;
    // Result: FluffOS is great!! :)

    Compare your results with: https://www.base64decode.org/
