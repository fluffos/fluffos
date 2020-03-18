---
layout: default
title: object / init
---

### 名称

    init - move_object() 函数呼叫对象中的 init() 方法以初始化指令(verb)和动作(action)

### 语法

    void init( void );

### 描述

    当 MUDLIB 移动对象 `A` 进入对象 `B`时，游戏驱动的 move_object() 外部函数会做以下行为：

    1. 如果 `A` 是生物(living)，让 `A` 呼叫 `B` 的 init() 方法。
    2. 不管 `A` 是否是生物，让 `B` 中的所有生物呼叫 `A` 的 init() 方法。
    3. 如果 `A 是生物，让 `A 呼叫 `B` 中所有对象的 init() 方法。

    注意：如果一个对象呼叫过 enable_commands() 外部函数，就被认为是生物。

    通常，对象中的 init() 方法用来呼叫 add_action()增加对象自身提供的指令(command)。

### 参考

    reset(4), move_object(3), enable_commands(3), living(3), add_action(3)

### 翻译

    雪风(i@mud.ren)
