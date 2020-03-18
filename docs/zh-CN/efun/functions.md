---
layout: default
title: contrib / functions.pre
---

### 名称

    functions() - 返回对象中定义的局部函数(方法)

### 语法

    mixed *functions( object ob );
    mixed *functions( object ob, int flag );

### 描述

    函数返回指定对象 `ob` 中定义的局部函数(方法)信息(包括继承对象中的方法)，可选参数 `flag` 为位域标识符，如果不指定默认为 0，表示只返回函数名称列表，如果指定为 1，返回一个二维数组，子数组格式为：

    ({ 函数名, 参数个数, 返回值类型, 参数类型 })

### 参考

    variables(3)

### 翻译 ###

    雪风(i@mud.ren)
