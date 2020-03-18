---
layout: default
title: contrib / program_info.pre
---

### 名称

    program_info() - 返回游戏对象的相关程序资料

### 语法

    mapping program_info(object ob | void);

### 描述

    返回包括程序各种信息的映射，格式如下：
    ([
        header size : int
        code size : int
        function size : int
        var size : int
        class size : int
        inherit size : int
        saved type size : int
        total size : int
    ])

    如果不指定对象，默认为所有游戏对象。

    此外部函数要求驱动程序编译时定义了 PACKAGE_CONTRIB 包。

### 参考

    debug_info(3), mud_status(3), rusage(3)

### 翻译

    雪风(i@mud.ren)
