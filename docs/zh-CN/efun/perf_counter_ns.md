---
layout: default
title: system / perf_counter_ns
---

### 名称

    perf_counter_ns() - 给出了以纳秒为单位的新纪元时间测量（时间戳）

### 语法

    int perf_counter_ns( void );

### 描述

    返回自1970年1月1日午夜（GMT）以来的纳秒数。.

### 参考

    ctime(3), localtime(3), time_expression(3), uptime(3), time(3)
