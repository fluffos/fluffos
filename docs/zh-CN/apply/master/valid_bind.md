---
layout: default
title: master / valid_bind
---

### 名称

    valid_bind - 控制外部函数（efun） bind() 的使用

### 语法

    int valid_bind(object binder, object old_owner, object new_owner)

### 描述

    当 binder 尝试使用 bind() 外部函数把一个函数指针从 old_ower 绑定到 new_owner 时，会呼叫此方法，返回1为允许，返回0为禁止。

### 参考

    bind(3)

### 翻译 ###

    雪风(i@mud.ren)
