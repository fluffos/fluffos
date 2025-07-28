---
layout: doc
title: strings / hash
---
# hash

### NAME

    hash() - hash a string with the specified algorithm

### SYNOPSIS

    string hash( string algo, string str );

### DESCRIPTION

    Returns the hash of string `str` by the `algo` algorithm.
    
    Algorithm `algo` can be one of:
    
    **Legacy algorithms:**
    md4 md5 ripemd160
    
    **Legacy algorithms (OpenSSL 1.x - 2.x only):**
    md2 mdc2
    
    **SHA-1 and SHA-2 family:**
    sha1 sha224 sha256 sha384 sha512
    
    **SHA-3 family (recommended):**
    sha3-224 sha3-256 sha3-384 sha3-512
    
    **BLAKE2 family (high performance):**
    blake2s256 blake2b512
    
    **Other algorithms:**
    sm3

    This requires PACKAGE_CRYPTO to be enabled when compiling the driver.
    
    **Compatibility Notes:**
    - MD2 and MDC2 are only available with OpenSSL 1.x - 2.x (removed in OpenSSL 3.x)
    - BLAKE2 family requires OpenSSL 1.1.0+
    - SHA-3 family and SM3 require OpenSSL 1.1.1+
    
    Note: For new applications, SHA-3 and BLAKE2 algorithms are recommended over 
    legacy MD and older SHA algorithms for better security and performance.

### EXAMPLE

    // Legacy algorithms
    hash("md4", "Something")        =   "abc554cae9acd8f168101954383335df"
    hash("md5", "Something")        =   "73f9977556584a369800e775b48f3dbe"
    hash("sha1", "Something")       =   ...40 characters...
    
    // Legacy algorithms (OpenSSL 1.x - 2.x only)
    hash("md2", "Something")        =   ...32 characters... (if available)
    hash("mdc2", "Something")       =   ...32 characters... (if available)
    
    // SHA-2 family
    hash("sha224", "Something")     =   ...56 characters...
    hash("sha256", "Something")     =   ...64 characters...
    hash("sha384", "Something")     =   ...96 characters...
    hash("sha512", "Something")     =   ...128 characters...
    
    // SHA-3 family (recommended for new applications)
    hash("sha3-256", "Something")   =   ...64 characters...
    hash("sha3-512", "Something")   =   ...128 characters...
    
    // BLAKE2 family (high performance)
    hash("blake2s256", "Something") =   ...64 characters...
    hash("blake2b512", "Something") =   ...128 characters...
    
    // Other algorithms
    hash("ripemd160", "Something")  =   ...40 characters...
    hash("sm3", "Something")        =   ...64 characters...

### SEE ALSO

    crypt(3), oldcrypt(3), sha1(3)