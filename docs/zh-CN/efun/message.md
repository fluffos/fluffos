---
layout: default
title: interactive / message
---

### 名称

    message() - 发送信息给活着的（living）对象

### 语法

    void message( mixed class, string message, mixed target );
    void message( mixed class, string message, mixed target, mixed exclude );

### 描述

    message() 呼叫 target 参数列表中除 exclude 参数列表以外的所有对象的 receive_message(mixed class, string message) 方法，通过此方法向对象发送消息。

    参数 `class` 是消息的类型（用于客户端消息处理），例如：`combat`，`shout`, `emergency`等。

    参数 `message` 是需要发送的消息字符串。

    参数 `target` 是需要接收消息的对象列表，此参数数据类型既可以是单一对象字符串或者对象指针，也可以是一个对象数组。如果对象是非活着的，那么在此对象环境中的所有对象都会收到信息。

    可选参数 `exclude` 是不需要接收消息的对象列表，此参数数据类型可以是对象或对象数组。

### 参考

    say(3), write(3), shout(3), tell_object(3), tell_room(3)

### 翻译

    雪风(i@mud.ren)
