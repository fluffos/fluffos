---
layout: default
title: filesystem / write_bytes
---

### 名称

    write_bytes() - 将一段连续的字节内容写入文件

### 语法

    int write_bytes( string path, int start, string series );

### 描述

    这个函数把字符串 `series` 的内容从文件 `path` 的字节位置 `start` 开始写入到文件。写入失败返回 0 ，成功返回 1 。

### 参考

    write_file(3), read_bytes(3)

### 翻译

    雪风(i@mud.ren)
