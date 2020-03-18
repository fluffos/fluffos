---
layout: default
title: filesystem / read_bytes
---

### 名称

    read_bytes() - 从一个文件读取连续的字节内容到一个字符串

### 语法

    string read_bytes( string path );
    string read_bytes( string path, int start );
    string read_bytes( string path, int start, int length );

### 描述

    这个函数从文件 `path` 中 `start` 字节位置开始读取 `length` 长度的字节，读取的字符以字符串类型返回。注意： (start + length) 的大小不可以超出文件大小，否则会读取失败，如果缺省第三个参数会从指定位置读到文件尾，如果后两个参数都缺省会读取整个文件。

### 参考

    read_file(3), write_bytes(3)

### 翻译

    雪风(i@mud.ren)
