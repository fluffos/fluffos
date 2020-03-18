---
layout: default
title: master / valid_object
---

### 名称

    valid_object - 允许控制可以加载哪些对象

### 语法

    int valid_object( object obj );

### 描述

    当加载一个对象后，驱动程序会以最新加载的对象为参数调用主控对象中的 valid_object() 方法。这个方法配合 `nomask` 修饰符和 inherits() 外部函数，可以安全的通过模拟外部函数限制 destruct() 的使用。

    如果方法存在且返回 0 ，对象会被销毁，并导致对象加载外部函数会出错。如果方法不存在或者返回 1 ，加载会正常进行。

    这个方法执行时，对象 `obj` 还没有执行包括 create() 方法在内的任何代码。

### 参考

    valid_override(4)

### 翻译 ###

    雪风(i@mud.ren)
