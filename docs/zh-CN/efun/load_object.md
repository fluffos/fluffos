---
layout: default
title: objects / load_object
---

### 名称

    load_object() - 通过文件名查找或载入一个对象

### 语法

    object load_object( string str );

### 描述

    通过文件名 `str` 查找游戏对象并返回对象指针，如果文件存在但对象未载入，载入并返回对象指针。如果对象不存在，返回 0。

### 参考

    file_name(3), stat(3), find_object(3)

### 翻译

    雪风(i@mud.ren)
