---
layout: default
title: objects / children
---

### 名称

    children() - 返回包含指定对象的所有复制对象的数组

### 语法

    object *children( string name );

### 描述

    这个外部函数返回一个文件名为 `name` 的对象及其所有复制对象组成的数组。一个使用 children() 的示例是查找所有玩家对象：

    object *list;

    list = children("/system/object/user");

    这让你能找到所有玩家对象（包括在线玩家和断线玩家，而 users() 外部函数仅报告在线玩家）。

### 参考

    deep_inherit_list(3), inherit_list(3), objects(3)

### 翻译

    雪风(i@mud.ren)
