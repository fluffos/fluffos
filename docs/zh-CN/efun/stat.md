---
layout: default
title: filesystem / stat
---

### 名称

    stat() - 返回指定文件或目录的有关信息

### 语法

    mixed stat( string str, int default: 0 );

### 描述

    如果 `str` 是一个文件，stat() 返回一个关于文件信息的数组，数组格式如下：

    ({ 文件大小, 文件最后修改时间, 文件载入游戏时间 })

    如果 `str` 是一个目录，或者第二个参数是 -1 ，stat() 的作用和 get_dir() 一样。

### 参考

    get_dir(3), file_size(3)

### 翻译

    雪风(i@mud.ren)
