---
layout: default
title: interactive / input_to
---

### 名称

    input_to()  - 把玩家接下来输入的内容传递给指定的函数

### 语法

    varargs void input_to( string | function fun, int flag, ... );

### 描述

    把玩家接下来输入的内容做为参数传给局部函数 `fun`。驱动程序不会解析（parse）输入的字符。

    请注意： input_to() 是非阻塞式监听，也就是说呼叫 input_to() 的对象不会暂停下来等待用户输入，而是继续执行后续程序。

    如果在同一次执行时多次调用 input_to()，仅第一次呼叫有效。

    如果可选参数 `flag` 非零，用户输入的字符不会回显，在被窃听(snoop)时也不会显示（这在输入密码时很有用）。

    函数 `fun` 执行时用户输入的字符会做为第一个参数（字符串类型），input_to() 函数 `flag` 之后的参数会做为额外参数传递给 `fun`。

    说明：这个外部函数的用法和 get_char() 外部函数及其类似，只是获取的输入从单个字符改为字符串。

### 参考

    call_other(3), call_out(3), get_char(3)

### 翻译

    雪风(i@mud.ren)
