---
layout: default
title: contrib / classes.pre
---

### 名称

    classes() - 返回指定对象中的 class 变量信息

### 语法

    mixed *classes( object ob );
    mixed *classes( object ob, int flag );

### 描述

    返回对象 `ob` 中的所有 class 类型变量数组，如果指定参数 `flag` 值为 1，还会返回成员变量名和变量类型，具体格式如下：

    （{ ({ “class_name”, ({ “member_name”, “type” }), … }), … })

### 参考

    functions(3), variables(3)

### 翻译

    雪风(i@mud.ren)
