---
layout: default
title: master / valid_write
---

### 名称

    valid_write - 检查某个用户是否对文件具有写权限

### 语法

    int valid_write( string file, mixed user, string func );

### 描述

    每当一个用户试图写入文件时，驱动程序调用主控对象中的 valid_write() 方法检查是否允许写入。参数 `file` 是要写入的文件名，参数 `user` 是尝试写入文件的用户，参数 `func` 是写入函数（一般是 write_file()）。如果主控对象中 valid_write() 方法不存在或者返回 1，允许写入，如果主控对象中 valid_write() 方法存在且返回 0 ，禁止写入。

### 参考

    valid_read(4)

### 翻译 ###

    雪风(i@mud.ren)
