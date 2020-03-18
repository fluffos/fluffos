---
layout: default
title: mappings / keys
---

### 名称

    keys() - 返回一个映射([key1:value1,key2:value2,...])的所有 key 组成的数组。

### 语法

    mixed *keys( mapping m );

### 描述

    keys() 返回一个包括映射 `m` 的所有索引（key）的数组。

    示例，如果:

    mapping m;
    m = (["hp" : 35, "sp" : 42, "mass" : 100]);

    那么

    keys(m) == ({"hp", "sp", "mass"})

    注意：返回的 key 并不会有固定的顺序，不过其顺序和 values() 外部函数返回的值顺序相同。

### 参考

    values(3), each(3)

### 翻译

    雪风(i@mud.ren)
