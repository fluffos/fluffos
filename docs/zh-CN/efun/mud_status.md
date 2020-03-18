---
layout: default
title: internals / mud_status
---

### 名称

    mud_status() - 报告驱动程序和 mudlib 的各种统计信息

### 语法

    string mud_status( int extra );

### 描述

    这个外部函数返回驱动程序和 mudlib 的统计信息。如果参数 `extra` 不为零，会输出更多附加信息。

### 参考

    debug_info(3), dumpallobj(3), memory_info(3), uptime(3)

### 翻译

    雪风(i@mud.ren)
