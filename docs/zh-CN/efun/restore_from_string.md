---
layout: default
title: contrib / restore_from_string.pre
---

### 名称

    restore_from_string() - 从一个字符串读取变量值到当前对象

### 语法

    void restore_from_string(string str, int flag);

### 描述

    这个外部函数到作用和 restore_object 类似， 唯一的区别是不是从文件读取存档，而是直接从存档字符串 `str` 中读取。

### 参考

    restore_object(3)

### 翻译

    雪风(i@mud.ren)
