---
layout: default
title: string / sha1
---

### 名称

    sha1() - 返回一个字符串的 sha1 哈希值

### 语法

    string sha1( string str );
    string sha1( buffer bf );

### 描述

    返回一个字符串 `str` 或 buffer `bf` 的 sha1 哈希值。

    这是 FLUFFOS 新增的外部函数，在编译驱动时需启用 PACKAGE_SHA1 。

### 翻译 ###

    雪风(i@mud.ren)
