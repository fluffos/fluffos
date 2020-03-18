---
layout: default
title: buffers / allocate_buffer
---

### 名称

    allocate_buffer() - 分配内存给一个缓冲区

### 语法

    buffer allocate_buffer( int size );

### 描述

    分配一个有 `size` 个元素的缓冲区。元素的数值必须 >= 0 而且不能超过系统上限（FLUFFOS 默认是 400000），所有元素初始化为 0 。

### 参考

    bufferp(3), read_buffer(3), write_buffer(3), sizeof(3), to_int(3)

### 作者

    Truilkan

### 翻译 ###

    雪风(i@mud.ren)
