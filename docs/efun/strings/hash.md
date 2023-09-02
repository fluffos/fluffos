---
layout: default
title: strings / hash
---

### NAME

    hash() - hash a string with the specified algorithm

### SYNOPSIS

    string hash( string algo, string str );

### DESCRIPTION

    Returns the hash of string `str` by the `algo` algorithm.
    
    Aglorithm `algo` can be one of:
    md4 md5 sha1 sha224 sha256 sha384 sha512 ripemd160

    This requires PACKAGE_CRYPTO to be enabled when compiling the driver.

### EXAMPLE

    hash("md4", "Something")        =   "abc554cae9acd8f168101954383335df"
    hash("md5", "Something")        =   "73f9977556584a369800e775b48f3dbe"
    hash("ripemd160", "Something")  =   ...40 characters...
    hash("sha1", "Something")       =   ...40 characters...
    hash("sha224", "Something")     =   ...56 characters...
    hash("sha256", "Something")     =   ...64 characters...
    hash("sha384", "Something")     =   ...96 characters...
    hash("sha512", "Something")     =   ...128 characters...

### SEE ALSO

    crypt(3), oldcrypt(3), sha1(3)