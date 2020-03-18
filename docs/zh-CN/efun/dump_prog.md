---
layout: default
title: internals / dump_prog
---

### 名称

    dump_prog() - 导出/反汇编一个 LPC 对象

### 语法

    void dump_prog( object ob );
    void dump_prog( object ob, int flags, string file );

### 描述


    dump_prog() 导出对象 `ob` 的程序资料到文件 `file`，如果没有文件的写入权限会导出失败，如果没有指定导出文件，会导出到文件 /PROG_DUMP。

    标识符 `flag` 默认值为 0，如果指定为 1，导出资料包括反汇编的信息，如果指定为 2，导出资料包括行号信息。

### 参考

    debug_info(3), dumpallobj(3)

### 翻译

    雪风(i@mud.ren)
