---
layout: default
title: internals / cache_stats
---

### 名称

    cache_stats() - 报告驱动程序和 mudlib 的统计资料

### 语法

    string cache_stats( void );

### 描述

    这个外部函数仅在驱动程序编译时定义过了 CACHE_STATS 时有效。这个函数会输出 call_other() 的缓存信息。

### 参考

    opcprof(3), mud_status(3)

### 翻译 ###

    雪风(i@mud.ren)
