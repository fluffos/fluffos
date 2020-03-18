---
layout: default
title: internals / query_load_average
---

### 名称

    query_load_average() - 报告 mud 的系统平均负载(load average)

### 语法

    string query_load_average( void );

### 描述

    这个外部函数返回一个报个每秒用户指令数和每秒驱动程序编译行数的信息字符串。

### 参考

    rusage(3)

### 翻译

    雪风(i@mud.ren)
