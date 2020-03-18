---
layout: default
title: interactive / add_action
---

### 名称

    add_action() - 绑定一个玩家指令到自定义函数

### 语法

    void add_action( string | function fun, string | string * cmd);
    void add_action( string | function fun, string | string * cmd, int flag );

### 描述

    当玩家键入的指令匹配 `cmd` 时，呼叫局部函数 `fun`，玩家指令的参数会做为字符串参数传给函数 `fun`，如果指令错误必须返回0，否则 `fun` 必须返回1。

    如果第二个参数 `cmd` 是一个数组，所有在数组中的指令都会呼叫函数 `fun`，你可以使用 query_verb() 外部函数找到呼叫函数的指令。

    如果指令错误，会继续查找其它命令，直到返回 true 或错误信息给玩家。

    通常 add_action() 只会从 init() apply 方法中调用，而且定义命令的对象必须是玩家可以接触到的，或者是玩家本身，或者被玩家携带，或者是玩家所在的房间，或者是玩家所在房间中的其它对象。

    如果不指定参数 `flag`，默认为0，代表输入指令必须完全匹配 `cmd` 才可生效，如果参数 `flag` 大于 0，只需玩家输入的指令前面部分匹配 `cmd` 即可生效。其中如果 `flag` 是 1 时 query_verb() 会返回输入的完整指令，如果参数 `flag` 是 2 时 query_verb() 会返回匹配部分以后的内容，而且这两种模式下获取的参数也不一样。

### 参考

    query_verb(3), remove_action(3), init(4)

### 翻译 ###

    雪风(i@mud.ren)
