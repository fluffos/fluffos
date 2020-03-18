---
layout: default
title: mudlib / export_uid
---

### 名称

    export_uid() - 设置指定对象的用户ID（uid）

### 语法

    int export_uid( object ob );

### 描述

    把对象 `ob` 的用户ID（uid）设置为当前对象的有效用户ID（euid）。这个函数只有对象 `ob` 的有效用户ID（euid）为 0 时才有效。

### 参考

    this_object(3), seteuid(3), getuid(3), geteuid(3), previous_object(3), valid_seteuid(4)

### 翻译

    雪风(i@mud.ren)
