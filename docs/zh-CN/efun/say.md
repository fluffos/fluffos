---
layout: default
title: interactive / say
---

### 名称

    say() - 把信息发送给在相同环境中的所有用户

### 语法

    void say( string str );
    void say( string str, object obj );
    void say( string str, object *obj );

### 描述

    把信息发送给发送者所在环境中的所有对象及所在环境是发送者的所有对象。发送者只能是 `this_player()` 或者 `this_player() == 0` 的 `this_object()`。

    第二个参数是可选的，如果指定第二个参数 `obj`，信息不会发给 `obj`，`obj` 可以是一个对象或者是对象数组。

### 参考

    message(3), write(3), shout(3), tell_object(3), tell_room(3)

### 翻译

    雪风(i@mud.ren)
