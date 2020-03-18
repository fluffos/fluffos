---
layout: default
title: system / function_exists
---

### 名称

    function_exists() - 在一个对象中寻找包含指定函数的文件

### 语法

    string function_exists( string str, object ob );
    string function_exists( string str, object ob, int flag );

### 描述

    返回对象 `ob` 中定义函数 `str` 的对象的文件名。如果函数由对象 `ob` 继承的对象定义，返回值会和 `file_name(ob)` 不同（返回的是被继承的对象文件）。

    如果函数在指定对象中没有定义，返回 0 。

    请注意，function_exists() 不会在投影对象 `ob` 的对象中寻找，也不会在无法从对象以外呼叫的函数（protected 和 private 类型）中寻找，但是如果指定参数 `flag` 且为非零值，protected 和 private 类型的方法也会被寻找到。。

### 参考

    call_other(3), call_out(3), functionp(3), valid_shadow(4)

### 翻译 ###

    雪风(i@mud.ren)
