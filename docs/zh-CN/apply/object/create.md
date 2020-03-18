---
layout: default
title: object / create
---

### 名称

    create - 对象初始化

### 语法

    void create( void );
    void create( ... );

### 描述

    所有对象都应该定义 create() 方法，所有对象在创建时都会调用 create() 方法完成初始化。

    对象在初始化时可以通过 new() 或 clone_object() 函数（efun）传递参数，create() 方法中的参数相对 new() 只是少了文件名。如：执行 new("/npc", "xuefeng", 18); 会在 npc 创建时调用 create("xuefeng", 18);

### 参考

    reset(4), __INIT(4)

### 作者

    Wayfarer@Portals

### 翻译

    雪风(i@mud.ren)
