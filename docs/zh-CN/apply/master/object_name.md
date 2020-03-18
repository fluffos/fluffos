---
layout: default
title: master / object_name
---

### 名称

    object_name - 由驱动程序调用以查找对象的名称

### 语法

    string object_name( object ob );

### 描述

    当外部函数 sprintf() 打印一个对象当值时会调用主控对象中的这个方法。这个方法应该返回一个代表对象名称的字符串(如用户名)。

### 参考

    file_name(3)

### 翻译

    雪风(i@mud.ren)
