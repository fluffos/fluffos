---
layout: default
title: strings / string_decode
---

### 名称

    string_decode() - 将缓冲区解码为 utf-8 字符串

### 语法

    string string_decode(buffer buf, string encoding);

### 描述

    将给定编码 `encoding` 的给定缓冲区 `buf` 解码为UTF-8字符串。

### 参考

    string_encode(3), buffer_transcode(3)
