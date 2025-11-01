---
layout: doc
title: string / hash
---
# hash

### 名称

    hash() - 返回一个字符串指定算法的哈希值

### 语法

    string hash(string algo, string str)

### 描述

    返回字符串 `str` 通过 `algo` 算法的哈希值。
    
    算法 `algo` 可以是以下之一：
    
    **传统算法：**
    md4 md5 ripemd160
    
    **传统算法（仅限 OpenSSL 1.x - 2.x）：**
    md2 mdc2
    
    **SHA-1 和 SHA-2 系列：**
    sha1 sha224 sha256 sha384 sha512
    
    **SHA-3 系列（推荐）：**
    sha3-224 sha3-256 sha3-384 sha3-512
    
    **BLAKE2 系列（高性能）：**
    blake2s256 blake2b512
    
    **其他算法：**
    sm3

    这是 FLUFFOS 新增的外部函数，需要在编译驱动时启用 PACKAGE_CRYPTO 。
    
    **兼容性说明：**
    - MD2 和 MDC2 仅在 OpenSSL 1.x - 2.x 中可用（在 OpenSSL 3.x 中已移除）
    - BLAKE2 系列需要 OpenSSL 1.1.0+
    - SHA-3 系列和 SM3 需要 OpenSSL 1.1.1+
    
    注意：对于新应用程序，建议使用 SHA-3 和 BLAKE2 算法，而不是传统的 MD 和较旧的 SHA 算法，以获得更好的安全性和性能。

### 翻译 ###

    雪风(i@mud.ren)
