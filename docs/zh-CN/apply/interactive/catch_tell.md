---
layout: default
title: interactive / catch_tell
---

### 名称

    catch_tell - 提供与用户交互的界面

### 语法

    void catch_tell( string message );

### 描述

    如果 MUDOS 编译是定义了 INTERACTIVE_CATCH_TELL 或者 fluffos 的运行时配置文件中设置了 `interactive catch tell` 值为 1，无论驱动程序以任何方式向对象发送信息（say()、shout()、tell_object()等等）玩家对象中的 catch_tell() 方法都会以消息 `message` 为参数被调用，消息可以以任何需要的方式被显示、删除或修改。这个方法可以灵活的处理消息是否显示、如何显示。

    请注意，哪怕没有启用玩家对象的 catch_tell() 方法，如果使用 tell_object() 外部函数发消息给非玩家对象，也会触发对象中的 catch_tell() 方法。

### 参考

    message(3), receive(3), receive_message(4)

### 翻译 ###

    雪风(i@mud.ren)
