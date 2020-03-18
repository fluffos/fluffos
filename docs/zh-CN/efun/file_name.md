---
layout: default
title: objects / file_name
---

### 名称

    file_name() - 取得对象的文件名

### 语法

    string file_name( object ob );

### 描述

    file_name() 外部函数返回对象 `ob` 载入时的文件名字符串，如果对象是复制对象，file_name() 返回的不是磁盘上的实际文件名，而是文件名后附加 `#` 和复制对象的编号。在游戏启动后，复制对象的编号从 0 开始，每复制一个对象编号加 1 ，所以每个复制对象的编号都是唯一的。

    如果没有指定参数 `ob`，默认对象是 this_object()。

### 翻译

    雪风(i@mud.ren)
