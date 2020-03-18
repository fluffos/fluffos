---
layout: default
title: objects / restore_object
---

### 名称

    restore_object() - 从一个文件读取变量值到当前对象

### 语法

    int restore_object( string name, int flag );

### 描述

    从文件 `name` 读取存档（变量和值）到当前对象。如果第二个可选参数是 1，所有不是 nosave 类型的变量不会在恢复存档前被重置为 0（正常情况下会重置为 0）。

    如果读取时出错，所有受影响到变量都不会被更改并传回错误。

### 参考

    save_object(3)

### 翻译

    雪风(i@mud.ren)
