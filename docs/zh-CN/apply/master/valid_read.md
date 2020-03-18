---
layout: default
title: master / valid_read
---

### 名称

    valid_read - 检查某个用户是否具有文件的读取权限

### 语法

    int valid_read( string file, mixed user, string func );

### 描述

    每当有用户试图读取文件，驱动程序都会调用主控对象中的 valid_read() 方法检查是否允许读取。参数 `file` 是准备读取的文件的文件名，参数 `user` 是尝试读取文件的用户，参数 `func` 是触发读取的函数。如果主控对象中 valid_read() 方法不存在或者返回 1，允许读取，如果主控对象中 valid_read() 方法存在且返回 0 ，禁止读取。

### 参考

    valid_write(4)

### 翻译 ###

    雪风(i@mud.ren)
