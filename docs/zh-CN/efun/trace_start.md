---
layout: default
title: system / trace_start
---

### 名称

    trace_start - 开始追踪驱动运行信息

### 语法

    void trace_start(filename, auto_stop_sec = 30)

### 描述

    调用这个函数开始收集追踪信息，包括LPC函数级别的运行资料。

    驱动在调用 `trace_end()` 或 `auto_stop_sec` 秒后停止收集并输出信息到 `filename` 文件，
    `auto_stop_sec` 默认为 30 秒。

    注意: 不要长时间的启动追踪，否则你会内存不足。

### 参考

    trace_end()
