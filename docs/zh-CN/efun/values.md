---
layout: default
title: mappings / values
---

### 名称

    values() - 返回一个映射([key1:value1,key2:value2,...])所有 value 组成的数组

### 语法

    mixed *values( mapping m );

### 描述

    values() 返回一个映射 `m` 所有 value 组成的数组

    示例，如:

    mapping m;

    m = (["hp" : 35, "sp" : 42, "mass" : 100]);

    那么

    values(m) == ({35, 42, 100})

    注意：返回的 value 的顺序和 keys() 外部函数返回的所有 key 的顺序一致。


### 参考

    keys(3), each(3)

### 翻译

    雪风(i@mud.ren)
