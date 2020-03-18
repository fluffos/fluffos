---
layout: default
title: objects / tell_object
---

### 名称

    tell_object() - 向一个对象发送消息

### 语法

    void tell_object( object ob, string str );

### 描述

    向 `ob` 对象发送消息 `str`，如果对象是玩家，消息会显示给对方，否则会被此对象中的 apply 方法 `catch_tell()` 捕获。

### 参考

    message(3), write(3), shout(3), say(3), tell_room(3)

### 翻译

    雪风(i@mud.ren)
