---
layout: default
title: buffers / buffer_transcode
---

### 名称

    buffer_transcode() - 转换缓冲区编码

### 语法

    buffer buffer_transcode( buffer src, string from_encoding, string to_encoding );

### 描述

    将给定缓冲区从编码 `from_encoding` 转换为编码 `to_encoding`。

### 参考

    string_encode(3), string_decode(3)
