---
layout: default
title: strings / strcmp
---

### 名称

    strcmp() - 检测两个字符串的大小关系

### 语法

    int strcmp( string one, string two );

### 描述

    这个外部函数的功能和C语言中的同名库函数完全相同。如果字符串 `one` 小于字符串 `tow` 返回值小于零，如果相对，返回 0 ，否则返回值大于零（大小关系按字母顺序，也是按其 ascii 值的大小比较）。

    这个外部函数在 sort_array() 中做为比较函数使用特别方便。

### 参考

    sort_array(3)

### 翻译

    雪风(i@mud.ren)
