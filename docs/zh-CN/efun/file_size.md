---
layout: default
title: filesystem / file_size
---

### 名称

    file_size() - 获取文件的大小

### 语法

    int file_size( string file );

### 描述

    file_size() 返回文件 `file` 的大小（字节），如果文件不存在或你无权读取返回　-1，如果 `file` 是一个目录返回 -2。

### 参考

    stat(3), get_dir(3)

### 翻译

    雪风(i@mud.ren)
