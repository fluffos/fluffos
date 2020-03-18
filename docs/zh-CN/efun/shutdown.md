---
layout: default
title: system / shutdown
---

### 名称

    shutdown() - 关闭驱动程序

### 语法

    void shutdown( int how );

### 描述

    和系统崩溃而强制关闭驱动程序不同，这个函数以可控的方式关闭驱动程序。参数 `how` 指定驱动应该以哪种方式关闭，如果要彻底关闭驱动请使用参数 -1，否则为重启驱动（实际上没用）。

    很显然，shutdown() 是一个需要把关安全的外部函数，和 exec() 一样，可以重写一个同名模拟外部函数做安全校验。请确保在主控对象中设置 valid_override() 方法来防止使用 efun::shutdown() 跳过安全校验。

### 参考

    crash(4)

### 翻译 ###

    雪风(i@mud.ren)
