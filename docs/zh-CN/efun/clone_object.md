---
layout: default
title: objects / clone_object
---

### 名称

    clone_object() - 加载一个对象的复制

### 语法

    object clone_object( string name, mixed extra, ... );
    object new( string name, mixed extra, ... );

### 描述

    从文件 `name` 创建一个新的对象，并给予它一个唯一的编号（在文件名后附加#XXX，XXX是整个游戏中独一无二的数字编号）。如果系统中没有加载文件名为 `name` 的对象（称为蓝图对象），调用本函数时先载入蓝图对象然后再复制这个对象并返回复制的对象（驱动程序会给复制对象设置一个 O_CLONE 标志），复制对象和蓝图对象共享代码，但可以有不同的变量。如果指定额外参数 `extra`，会传给要加载的对象的 create() 方法。

    补充说明：复制对象传递参数的功能结合虚拟对象的功能可以很轻易的实现非常强大而有趣的功能（比如生成无限世界）。

### 参考

    destruct(3), move_object(3), new(3)

### 翻译

    雪风(i@mud.ren)
