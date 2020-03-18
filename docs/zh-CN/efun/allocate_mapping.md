---
layout: default
title: mappings / allocate_mapping
---

### 名称

    allocate_mapping() - 为一个映射（mapping）变量预分空间

### 语法

    mapping allocate_mapping( int size );

### 描述

    返回一个预分配了 `size` 个元素的映射变量。

    示例:

    mapping x;
    int y = 200;

    x = allocate_mapping(y);

    示例中 y 是映射 x 的初始大小。如果你已经确定映射有多少个元素，使用 allocate_mapping 初始化变量是首选方式。因为这样一次性分配所有空间的内存利用效率更高。如果你要使用映射存储 200 项资料，使用示例初始化是相当适合的。注意，以上示例并不代表只能存储 200 项资料，只是前 200 项资料能更有效的存储。

    如果你需要从映射中删除大量的元素，使用 x = ([]) 来初始化映射比使用 allocate_mapping() 更好。因为在这种情况下 `size` 是无意义的， x= allocate_mapping(200); 的效果等于 x =([ ]);

### 参考

    map_delete(3)

### 翻译

    雪风(i@mud.ren)
