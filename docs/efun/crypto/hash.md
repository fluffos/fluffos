---
layout: doc
title: crypto / hash
---
# hash

### NAME

    hash - compute cryptographic hash of a string

### SYNOPSIS

    string hash(string algorithm, string data);

### DESCRIPTION

    Computes a cryptographic hash of the input data using the specified algorithm.
    Returns the hash as a hexadecimal string.

    This efun utilizes the OpenSSL crypto library and supports a wide range of
    hash algorithms from legacy to modern cryptographic hash functions.

### SUPPORTED ALGORITHMS

**Legacy Hash Algorithms** (OpenSSL 1.0.0+):
- `"md2"` - MD2 (deprecated, removed in OpenSSL 3.0+)
- `"md4"` - MD4 (insecure, for legacy compatibility only)
- `"md5"` - MD5 (128-bit, insecure for cryptographic purposes)
- `"mdc2"` - MDC2 (deprecated, removed in OpenSSL 3.0+)
- `"sha1"` - SHA-1 (160-bit, deprecated for cryptographic use)
- `"ripemd160"` - RIPEMD-160 (160-bit)

**SHA-2 Family** (OpenSSL 1.0.0+):
- `"sha224"` - SHA-224 (224-bit)
- `"sha256"` - SHA-256 (256-bit, recommended for general use)
- `"sha384"` - SHA-384 (384-bit)
- `"sha512"` - SHA-512 (512-bit)

**SHA-3 Family** (OpenSSL 1.1.1+, FIPS 202 standard):
- `"sha3-224"` - SHA3-224 (224-bit)
- `"sha3-256"` - SHA3-256 (256-bit, recommended modern algorithm)
- `"sha3-384"` - SHA3-384 (384-bit)
- `"sha3-512"` - SHA3-512 (512-bit)

**BLAKE2 Family** (OpenSSL 1.1.0+, RFC 7693):
- `"blake2b512"` - BLAKE2b-512 (512-bit, very fast and secure)
- `"blake2s256"` - BLAKE2s-256 (256-bit, optimized for 8-32 bit platforms)

**Other Modern Algorithms**:
- `"sm3"` - SM3 (256-bit, Chinese national standard, OpenSSL 1.1.1+)

### ARGUMENTS

- `algorithm` - The hash algorithm name (case-insensitive)
- `data` - The string data to hash

### RETURN VALUE

Returns a hexadecimal string representation of the computed hash.

### ERRORS

Generates an error if the specified algorithm is unknown or not supported
by the OpenSSL version linked with FluffOS.

### EXAMPLES

```c
// Hash a password with SHA-256
string password_hash = hash("sha256", "my_password");
// Result: "d80e3a0e7c0c67c7e5e8c5c5e4c7c5e..."

// Use modern SHA-3
string secure_hash = hash("sha3-256", "important data");

// Use BLAKE2 for high performance hashing
string fast_hash = hash("blake2b512", large_data);

// Legacy MD5 (for compatibility only, not secure)
string md5_sum = hash("md5", "test data");
// Result: "eb733a00c0c9d336e65691a37ab54293"
```

### SECURITY NOTES

**Recommended Algorithms:**
- **General purpose**: `sha256`, `sha3-256`, or `blake2b512`
- **High performance**: `blake2b512` or `blake2s256`
- **Standards compliance**: `sha256` (FIPS 180-4) or `sha3-256` (FIPS 202)

**Deprecated/Insecure Algorithms:**
- **DO NOT USE** for security: `md2`, `md4`, `md5`, `sha1`
- These are provided only for legacy compatibility and non-cryptographic uses
- `md5` and `sha1` are vulnerable to collision attacks

**Version Requirements:**
- Legacy algorithms (MD4, MD5, SHA-1, SHA-2, RIPEMD160): OpenSSL 1.0.0+
- BLAKE2 family: OpenSSL 1.1.0+
- SHA-3 family and SM3: OpenSSL 1.1.1+
- MD2 and MDC2: Only available in OpenSSL versions prior to 3.0

### SEE ALSO

- crypt(3) - Password hashing with salts
- oldcrypt(3) - Legacy password hashing

### NOTES

- Algorithm names are case-insensitive
- The package must be compiled with PACKAGE_CRYPTO enabled
- Requires linking against OpenSSL crypto library (-lssl -lcrypto)
- On Windows, PACKAGE_CRYPTO is typically disabled by default

### AVAILABILITY

Requires the crypto package to be enabled at compile time.
Not available on all platforms (typically disabled on Windows).
