---
layout: default
title: string / hash
---

### 名称

    hash() - 返回一个字符串指定算法的哈希值

### 语法

    string hash(string algo, string str)

### 描述

    返回字符串 `str` 通过 `algo` 算法的哈希值，算法 `algo` 可以是 md4 md5 sha1 sha224 sha256 sha384 sha512 ripemd160

    这是 FLUFFOS 新增的外部函数，需要在编译驱动时启用 PACKAGE_CRYPTO 。

### 翻译 ###

    雪风(i@mud.ren)
