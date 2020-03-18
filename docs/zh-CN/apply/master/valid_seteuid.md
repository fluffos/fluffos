---
layout: default
title: master / valid_seteuid
---

### 名称

    valid_seteuid - 保护 seteuid() 外部函数的使用

### 语法

    int valid_seteuid( object obj, string euid );

### 描述

    驱动程序通过内置的 seteuid() 外部函数调用主控对象中的 valid_seteuid(ob, euid) 方法。如果 valid_seteuid() 返回 0 ，seteuid() 执行失败；如果 valid_seteuid() 返回 1 ，seteuid() 执行成功。

### 参考

    seteuid(3), geteuid(3), getuid(3), export_uid(3)

### 翻译 ###

    雪风(i@mud.ren)
