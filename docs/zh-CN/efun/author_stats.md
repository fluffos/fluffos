---
layout: default
title: mudlib / author_stats
---

### 名称

    domain_stats() - 返回从域上收集的统计信息
    author_stats() - 返回从创建者上收集的统计信息

### 语法

    mapping domain_stats( string domain | void );
    mapping author_stats( string domain | void );

### 描述

    domain_stats() 和 author_stats() 返回的都是信息映射。如果没有指定参数，返回的信息是所有域或所有创建者的。如果指定参数，返回的信息映射是指定域或创建者的，映射的键为：moves、errors、heart_beats、array_size 和 errors，值为整型数值。

    Moves is the number of objects that have moved into objects in the given domain（or author）. Errors is the number of errors incurred by objects with the given domain（or author）. Heart_beats is the number of heartbeat calls made on objects having the given domain（or author）. Array_size is the size (in bytes) of the arrays allocated by the domain（or author）. Objects is the number of objects created by the given domain（or author）.

    如果没有指定参数，返回的映射格式如下：

    ([ domain0 : info0, domain1 : info1, ... ])

    其中 info0 格式如下：

    ([ "moves" : moves, "errors" : errors, "heart_beats" : heart_beats, "array_size" : array_size, "objects" : objects ])

    如果指定参数，返回的映射格式和 info0 相同。

### 参考

    domain_file(4), author_file(4), set_author(3)

### 翻译

    雪风(i@mud.ren)
