---
layout: default
title: contrib / variables.pre
---

### 名称

    variables() - 返回对象中定义的全局变量信息

### 语法

    mixed *variables( object ob );
    mixed *variables( object ob, int flag );

### 描述

    函数返回指定对象 `ob` 中定义的全局变量信息(包括来自继承对象的全局变量)，可选参数 `flag` 为位域标识符，如果不指定默认为 0，表示只返回变量名称列表，如果指定为 1，返回一个二维数组，子数组格式为：

    ({ 变量名, 变量类型 })

### 参考

    functions(3)

### 翻译 ###

    雪风(i@mud.ren)
