---
layout: default
title: system / reclaim_objects
---

### 名称

    reclaim_objects - 清理残留的对象

### 语法

    int reclaim_objects( void );

### 描述

    重复检查所有已加载的对象，并释放残留的对象，这个能释放大量内存占用，具体取决于MUD的代码情况。如果一个对象仅仅是被已摧毁的对象的全局变量指针，这个对象就是残留的对象。这个函数返回值是被清理的对象的数量。

### 参考

    destruct(3)

### 翻译

    雪风(i@mud.ren)
