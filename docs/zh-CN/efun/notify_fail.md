---
layout: default
title: interactive / notify_fail
---

### 名称

    notify_fail - 传回一段错误讯息

### 语法

    int notify_fail( string | function str );

### 描述

    取代通过 add_action() 外部函数设置的默认错误消息，将 `str` 存为要返回的错误消息。这是一种比较好的显示错误消息的方式，因为这个函数允许其它对象继续响应这个指令。请不要使用 write() 显示错误消息，因为这样做会要求函数返回 1，除非你想在 write() 返回的消息后看到运行时配置文件中设置的 `default fail message`，而如果直接返回 1，又会让其它对象失去了响应这个指令的机会。

    如果参数使用函数指针，会调用这个函数，如果函数返回一个字符串，这个字符串会做为指令执行失败的消息。只要正确的设置 this_player()，在函数中也可以使用 write()。

    如果 notify_fail() 调用多次，仅最后一次呼叫有效。

    这个函数可以在指令执行失败时传回更友好的错误提示。

### 返回值

    notify_fail() 永远返回 0。

### 翻译

    雪风(i@mud.ren)
