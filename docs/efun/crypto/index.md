---
layout: doc
title: crypto
---

# Crypto Package

The crypto package provides cryptographic hash functions using OpenSSL.

## Functions

* [hash](hash.html) - Compute cryptographic hash of a string

## Overview

The crypto package utilizes the OpenSSL crypto library to provide various message digest hashes.
It supports legacy hash algorithms (MD4, MD5, SHA-1, RIPEMD160), the SHA-2 family
(SHA-224, SHA-256, SHA-384, SHA-512), and modern hash algorithms (SHA-3 family, BLAKE2, SM3).

## Availability

The crypto package must be enabled at compile time with `PACKAGE_CRYPTO=ON`.
This package requires OpenSSL to be installed.

**Note**: On Windows platforms, PACKAGE_CRYPTO is typically disabled by default.

## Version Compatibility

- **OpenSSL 1.0.0+**: Legacy algorithms (MD4, MD5, SHA-1, SHA-2, RIPEMD160)
- **OpenSSL 1.1.0+**: BLAKE2 family
- **OpenSSL 1.1.1+**: SHA-3 family and SM3
- **OpenSSL 3.0+**: MD2 and MDC2 removed/deprecated

## Security Recommendations

For new applications, use:
- **SHA-256** or **SHA3-256** for general cryptographic hashing
- **BLAKE2b512** or **BLAKE2s256** for high-performance hashing

Avoid using MD5, SHA-1, or other legacy algorithms for security-sensitive operations.
