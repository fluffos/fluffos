---
layout: default
title: filesystem / write_file
---

### 名称

    write_file() - 将字符串写入到文件中

### 语法

    int write_file( string file, string str, int flag );

### 描述

    将字符串 `str` 追加到文件 `file` 中，写入成功返回 1，失败返回  0 。如果参数 `flag` 是 1，写入方式是覆盖而不是追加。

### 参考

    read_file(3), write_buffer(3), file_size(3)

### 翻译

    雪风(i@mud.ren)
