---
layout: default
title: master / log_error
---

### 名称

    log_error - 灵活的记录错误

### 语法

    void log_error( string file, string message );

### 描述

    在编译过程中出现错误时，驱动程序以发生错误的对象的文件名 `file` 和 错误信息 `message` 为参数调用主控对象中的 log_error() 方法。之后，log_error() 方法可以根据这些信息自由的做任何它认为可以做的事情，通常，log_error() 会根据文件名决定错误信息应该记录在哪里并写入对应的文件。

### 参考

    error_handler(4)

### 作者

    Wayfarer@Portals

### 翻译

    雪风(i@mud.ren)
