---
layout: default
title: mudlib / getuid
---

### 名称

    getuid() - 返回对象的用户ID（uid）

### 语法

    string getuid( object ob | void );

### 描述

    返回对象的用户ID（uid），一个对象的用户ID（uid）由对象创建时 creator_file() 方法的返回值决定，这个方法在主控对象。如果不指定对象，默认对象为 this_object()。

### 参考

    seteuid(3), geteuid(3), export_uid(3), this_object(3), previous_object(3), creator_file(4), valid_seteuid(4)

### 翻译

    雪风(i@mud.ren)
