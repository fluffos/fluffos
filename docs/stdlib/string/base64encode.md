---
layout: default
title: stdlib / base64encode
---

### NAME

    base64encode - encode a string into base64

### SYNOPSIS

    string base64encode( string source ) ;

### ARGUMENTS

    source - the source string to encode

### DESCRIPTION

    Encodes and returns a string in base64.

### EXAMPLES

    printf( "%s", base64encode( "FluffOS is great!! :)" ) ) ;
    // Result: Rmx1ZmZPUyBpcyBncmVhdCEhIDop

    Compare your results with: https://www.base64encode.org/
