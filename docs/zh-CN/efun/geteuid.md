---
layout: default
title: mudlib / geteuid
---

### 名称

    geteuid() - 返回一个对象或函数的有效用户ID（euid）

### 语法

    string geteuid( object ob | function f | void );

### 描述

    如果参数是对象，返回对象的有效用户ID（euid），如果参数是一个函数指针，返回创建这个函数指针的对象的有效用户ID，如果在创建这个函数指针时对象没有 euid ，会使用对象的有效用户ID（uid）代替。如果不指定对象，默认对象为 this_object()。

### 参考

    seteuid(3), getuid(3), functionp(3), export_uid(3), previous_object(3), this_object(3), valid_seteuid(4)

### 翻译

    雪风(i@mud.ren)
