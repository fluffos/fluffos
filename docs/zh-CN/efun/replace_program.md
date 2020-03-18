---
layout: default
title: system / replace_program
---

### 名称

    replace_program() - 把当前对象替换为指定对象的复制对象

### 语法

    void replace_program( string str );

### 描述

    注意：此函数在 FluffOS 中已弃用。

    replace_program() replaces the program in this_object() with that of an object it inherits. The string argument is the filename of the object whose program is to be used. Once the replacement takes place, the current object effectively becomes a clone of that other object, but with its current filename and global variable values. The program is not actually replaced until the current execution is completed.

### 参考

    clone_object(3), new(3)

### 翻译

    雪风(i@mud.ren)
