---
layout: default
title: system / inherit_list
---

### 名称

    inherit_list() - 获取对象继承的对象列表

### 语法

    string *inherit_list( object obj );

### 描述

    返回对象 `obj` 继承的对象的文件名数组，仅仅返回直接继承的对象，如：A 继承 B，B 继承 C，inherit_list(A) 返回的数组中只有 B 没有 C。

### 参考

    deep_inherit_list(3), inherits(3)

### 翻译

    雪风(i@mud.ren)
