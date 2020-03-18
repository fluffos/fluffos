---
layout: default
title: calls / call_out
---

### 名称

    call_out() - 延迟呼叫同一对象中的函数

### 语法

    void call_out( string | function fun, int delay, mixed arg );

### 描述

    设置在当前对象（this_object()）中呼叫函数 `fun`，呼叫会延迟 `delay` 秒，`arg` 做为参数传给函数 `fun`。

    请注意：如果运行时配置文件中 THIS_PLAYER_IN_CALL_OUT 的值不为1，你将不能在 `fun` 中使用 write() 或 say()，这种情况下可以使用 tell_object() 代替。

### 参考

    remove_call_out(3), call_out_info(3)

### 翻译 ###

    雪风(i@mud.ren)
