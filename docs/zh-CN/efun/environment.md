---
layout: default
title: objects / environment
---

### 名称

    environment() - 返回对象的环境

### 语法

    object environment( object ob | void );

### 描述

    返回包含对象 `ob` 的对象(环境)，如果没有指定参数 `ob`，默认是 this_object()。如果对象 `ob` 不在任何环境里，返回 0。

### 参考

    this_object(3)

### 翻译

    雪风(i@mud.ren)
