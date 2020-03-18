---
layout: default
title: system / inherits
---

### 名称

    inherits() - 检测一个对象是否继承指定文件

### 语法

    int inherits( string file, object obj );

### 描述

    如果 `ob` 没有继承 `file` inherits() 返回 0，如果继承最新版 `file` 返回 1，如果继承旧版本 `file` 返回 2.

### 参考

    deep_inherit_list(3), inherit_list(3)

### 翻译

    雪风(i@mud.ren)
