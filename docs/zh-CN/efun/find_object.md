---
layout: default
title: objects / find_object
---

### 名称

    find_object() - 通过文件名（file_name()）查找对象

### 语法

    object find_object( string str );
    object find_object( string str, int flag);

### 描述

    通过文件名 `str` 查找游戏中已加载的对象。如果对象是复制对象，文件名可以通过 file_name() 外部函数取得。如果对象不存在或未载入游戏，返回 0，否则返回对象指针。

    如果可选参数 `flag` 为 1，此函数的效果等同于 load_object()。

### 参考

    file_name(3), stat(3), load_object(3)

### 翻译

    雪风(i@mud.ren)
