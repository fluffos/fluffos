---
layout: default
title: floats / to_int
---

### 名称

    to_int - 把一个浮点数或缓冲区转换成成整数

### 语法

    int to_int( float | buffer x );

### 描述

    如果 `x` 是一个浮点数，to_int() 返回小数点之前的整数。

    If 'x' is a    If 'x' is a buffer, the call returns the integer (in network-byte-order) that is embedded in the buffer.

### 参考

    to_float(3), read_buffer(3), sprintf(3), sscanf(3)

### 翻译 ###

    雪风(i@mud.ren)
