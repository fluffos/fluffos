---
layout: default
title: objects / tell_room
---

### 名称

    tell_room() - 对一个房间中的所有对象发送消息

### 语法

    void tell_room( mixed ob, string str );
    void tell_room( mixed ob, string str, object *exclude );

### 描述

    向房间 `ob` 中的所有对象发送消息 `str`，`ob` 既可以是房间对象，也可以是房间文件名字符串。如果指定 `exclude` 参数，所有在此数组中的对象不会收到消息。

### 参考

    message(3), write(3), shout(3), say(3), tell_object(3)

### 翻译

    雪风(i@mud.ren)
