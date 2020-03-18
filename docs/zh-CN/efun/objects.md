---
layout: default
title: objects / objects
---

### 名称

    objects - 返回包含游戏已加载的所有对象的数组

### 语法

    object *objects( void );
    object *objects( string func, object ob );
    object *objects( function f );

### 描述

    返回 MUD 游戏中加载的所有对象的数组。请注意，如果系统设置的数组大小最大值太小，objects() 返回的数组会被截断到允许的最大值，这种情况下，返回值也没什么用处了。

    如果指定可选参数 `func` 和 `ob`，所有已加载的对象都会作为参数呼叫 ob->func()，函数返回值为 0 的对象会被过滤掉。

    第三种用法和第二种类似，只是参数改为函数指针。如：objects( (: clonep :) ) 返回游戏中所有复制对象的列表。

### 参考

    livings(3), users(3)

### 翻译

    雪风(i@mud.ren)
