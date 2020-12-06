---
layout: default
title: contrib / fetch_class_member.pre
---

### 名称

    fetch_class_member

### 语法

    mixed fetch_class_member( mixed instantiated_class, int index );

### 描述

    返回实例化的类 `instantiated_class` 的索引为 `index` 的成员的值，这对数据成员没有标签（变量名）的匿名类非常有用。

### 示例

    mixed me = assemble_class( ({ "Foo", 42, "Fooville" }) ) ;
    int age ;
    string name, city ;

    name = fetch_class_member( me, 0 ) ;
    age = fetch_class_member( me, 1 ) ;
    city = fetch_class_member( me, 2 ) ;

### 参考

    store_class_member(3)

### 翻译

    雪风(i@mud.ren)
