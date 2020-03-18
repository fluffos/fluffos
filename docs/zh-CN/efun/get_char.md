---
layout: default
title: interactive / get_char
---

### 名称

    get_char - 把玩家下次输入的字符传递给指定的函数

### 语法

    varargs void get_char( string | function fun, int flag, ... );

### 描述

    把玩家接下来输入的字符做为参数传给函数 `fun`。驱动程序不会解析（parse）输入的字符。

    请注意： get_char() 是非阻塞式监听，也就是说呼叫 get_char() 的对象不会暂停下来等待用户输入，而是继续执行后续程序。

    如果在同一次执行时多次调用 get_char()，仅第一次呼叫有效。

    如果可选参数 `flag` 非零，用户输入的字符不会回显，在被窃听(snoop)时也不会显示（这在输入密码时很有用）。

    函数 `fun` 执行时用户输入的字符会做为第一个参数（字符串类型），get_char() 函数 `flag` 之后的参数会做为额外参数传递给 `fun`。

### 参考

    call_other(3), call_out(3), input_to(3)

### 翻译

    雪风(i@mud.ren)
