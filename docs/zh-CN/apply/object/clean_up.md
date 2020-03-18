---
layout: default
title: object / clean_up
---

### 名称

    clean_up - 在不活动(inactive)的对象中定期呼叫此方法

### 语法

    int clean_up( int inherited );

### 描述

    驱动程序会定期呼叫所有已经一段时间不活跃的对象中的 clean_up() 方法。这段时间的长短在运行时配置文件中指定。传递给函数的标识符 `inherited` 指明此对象是否被其它对象继承。

    如果 clean_up() 返回 0，此方法在这个对象中永远不再重复呼叫，如果返回 1，在指定的 clean_up() 延迟时间后对象还是不活动时，会再次呼叫 clean_up() 方法。

    通常对象使用此方法摧毁自己以节省内存，不过一般不会摧毁已经被其它对象继承的对象。

### 翻译

    雪风(i@mud.ren)
