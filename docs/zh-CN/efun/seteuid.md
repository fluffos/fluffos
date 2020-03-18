---
layout: default
title: mudlib / seteuid
---

### 名称

    seteuid() - 设置当前对象的有效用户ID（euid）

### 语法

    int seteuid( string str );

### 描述

    设置当前对象的有效用户ID为 `str`。主控对象中的 valid_seteuid() 方法控制了对象的 euid 可以被设置为什么。

    如果 euid 设置为 0，当前对象的用户ID（uid）可以且仅可以使用 export_uid() 外部函数改变。

    但是，如果 euid 为 0，这个对象不能载入或复制任何对象。

### 参考

    export_uid(3), getuid(3), geteuid(3), this_object(3), valid_seteuid(4)

### 翻译

    雪风(i@mud.ren)
