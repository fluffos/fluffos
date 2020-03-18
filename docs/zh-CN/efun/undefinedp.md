---
layout: default
title: general / undefinedp
---

### 名称

    undefinedp() - 检测指定变量是否未定义（undefined）

### 语法

    int undefinedp( mixed arg );

### 描述

    如果变量 `arg` 未定义返回 1。`arg` 在以下几种情况下算未定义：

    1. 变量是 call_other 外部函数呼叫对象中不存在的函数的返回值(如： arg = call_other(obj, "???"))
    2. 变量是映射变量中不存在的元素。(如： arg = map[not_there])
    3. 变量没有初始化。
    4. 变量是函数的形式参数，但在调用时没有传递实参。

### 参考

    mapp(3),  stringp(3),  pointerp(3),  objectp(3),  intp(3),  bufferp(3),
    floatp(3), functionp(3), nullp(3), errorp(3)

### 翻译

    雪风(i@mud.ren)
