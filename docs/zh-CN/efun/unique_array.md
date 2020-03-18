---
layout: default
title: arrays / unique_array
---

### 名称

    unique_array() - 将一个数组对象分组

### 语法

    mixed unique_array( object *obarr, string separator );
    mixed unique_array( object *obarr, string separator, mixed skip );
    mixed unique_array( mixed *arr, function f );
    mixed unique_array( mixed *arr, function f, mixed skip );

### 描述

    将 `separator` 函数返回值相同的对象分为一组，`obarr` 只能是对象数组。在 `obarr`中的每个对象都会呼叫 `separator` 函数一次，如果指定 `skip` 参数，会先过滤掉符合条件的对象元素。

    第二形式稍有不同，数组中的每个元素做为参数传给函数指针 f ，元素按函数返回值分组，但是数组不再限制为对象类型。

### 返回值

    返回值是有多个对象数组的二维数组，格式如下：

    ({
        ({Same1:1, Same1:2, Same1:3, .... Same1:N }),
        ({Same2:1, Same2:2, Same2:3, .... Same2:N }),
        ({Same3:1, Same3:2, Same3:3, .... Same3:N }),
        ....
        ....
        ({SameM:1, SameM:2, SameM:3, .... SameM:N }),
    })

### 翻译

    雪风(i@mud.ren)
