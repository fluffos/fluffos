---
layout: default
title: mappings / map_delete
---

### 名称

    map_delete() - 通过 key 从一个映射移除一组值(key:value)

### 语法

    void map_delete( mapping m, mixed element );

### 描述

    map_delete 从映射 `m` 中移除 key 为 `element` 的键值对(key : value)。

    示例，给定：

    mapping names;

    names = ([]);
    names["truilkan"] = "john";
    names["wayfarer"] = "erik";
    names["jacques"] = "dwayne";

    那么：

    map_delete(names,"truilkan");

    导致映射 `names` 的结果为：

    (["wayfarer" : "erik", "jacques" : "dwayne"])

    在 map_delete(names, "truilkan") 后映射 `names` 中不再包括 `truilkan`，除非再被加回去。

### 参考

    allocate_mapping(3)

### 翻译

    雪风(i@mud.ren)
