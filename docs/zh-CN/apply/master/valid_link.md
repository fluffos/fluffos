---
layout: default
title: master / valid_link
---

### 名称

    valid_link - 控制 link() 外部函数的使用

### 语法

    int valid_link( string from, string to );

### 描述

    驱动程序从 link(from, to) 外部函数调用主控对象中的 valid_link(from, to) 方法。如果 valid_link() 返回 0 ，link() 失败，如果返回 1 ，当用相同参数执行 rename(from, to) 也成功时 link() 成功。

### 参考

    link(3)

### 翻译 ###

    雪风(i@mud.ren)
