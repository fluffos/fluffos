---
layout: default
title: internals / dumpallobj
---

### 名称

    dumpallobj()  - 报告已加载的所有对象的各种统计信息

### 语法

    void dumpallobj( string filename | void );

### 描述

    这个外部函数导出已加载的所有对象的统计信息列表。如果没有指定参数，信息会导出到文件 /OBJ_DUMP，如果指定参数 `filename`，会导出信息到文件 `filename`。

### 参考

    mud_status(3), debug_info(3)

### 翻译

    雪风(i@mud.ren)
