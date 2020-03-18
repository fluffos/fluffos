---
layout: default
title: arrays / sort_array
---

### 名称

    sort_array() - 对数组排序

### 语法

    mixed *sort_array( mixed *arr, string fun, object ob );
    mixed *sort_array( mixed *arr, function f );
    mixed *sort_array( mixed *arr, int direction );

### 描述

    第一种形式返回一个和参数 `arr` 元素相同的数组，只是根据 `ob->fun()` 的规则按升序快速排序。`ob->fun()` 返回值 -1、0、1 说明二个元素的关系是小于、等于或大于。

    第二种形式和第一种形式功能相同，只是参数使用函数指针。

    第三种形式返回一个和参数 `arr` 元素相同的数组，只是使用内置排序程序快速排序，参数 `direction` 0或1代表升序，-1代表降序。使用内置排序程序的局限是数组元素必须是相同类型的，参与比较的元素必须是字符串、整型或浮点型中的一种，如果数组中包含数组，只会根据子数组的第一个元素进行排序。

### 参考

    filter_array(3), map_array(3), strcmp(3)

### 翻译

    雪风(i@mud.ren)
