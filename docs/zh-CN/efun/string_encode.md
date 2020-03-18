---
layout: default
title: strings / string_encode
---

### 名称

    string_encode() - 将 utf-8 字符串编码为缓冲区

### 语法

    buffer string_encode(string str, string encoding);

### 描述

    使用给定的编码 `encoding` 对 utf-8 编码的字符串 `str` 进行编码，返回字节缓冲区。

### 参考

    string_decode(3), buffer_transcode(3)
