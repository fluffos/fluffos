---
layout: default
title: internals / debug_info
---

### 名称

    debug_info() - 显示调试信息

### 语法

    string debug_info( int operation, object ob );

### 描述

    debug_info() 是一个用来调试驱动的通用工具。调试信息的操作由第一个参数 `operation` 决定，可用参数值为 0 和 1 ，第二个参数 `ob` 为要调试的对象。

### SEE ALSO

    dump_file_descriptors(3), dump_socket_status(3)

### 翻译 ###

    雪风(i@mud.ren)
