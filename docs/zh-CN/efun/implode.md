---
layout: doc
title: strings / implode
---
# implode

### 名称

    implode() - 将一个一维数组的值转化成字符串

### 语法

    string implode( mixed *arr, string del );
    mixed implode( mixed *arr, function f, void | mixed extra );

### 描述

    第一种形式：使用字符串 `del` 连接数组 `arr` 中的每个字符串并返回合并后的字符串。数组中仅字符串类型的数据会被处理，非字符串类型的元素会被忽略。

    第二种形式：当第二个参数为函数时，implode() 会对数组进行归并（fold）——从左到右依次调用函数 `f`，把当前的累积结果与下一个元素合并，并返回最后一次调用的结果。可选的 `extra` 参数会作为初始值。

### 返回值

    连接形式返回合并后的字符串；函数形式返回累积得到的值（可为任意类型）。

### 参考

    explode(3), sprintf(3)

### 翻译 ###

    雪风(i@mud.ren)
