---
layout: default
title: interactive / enable_commands
---

### 名称 ###

    enable_commands() - 允许对象使用玩家命令

### 语法 ###

    已弃用:
    void enable_commands( void );

    从 FluffOS 3.0-alpha7.1 开始:
    void enable_commands( int setup_actions = 0 );

### 描述 ###

    enable_commands() 把 this_object() 标记为活的（living）对象（生物），并允许对象通过 command() 外部函数使用由 add_action() 外部函数增加的命令。当对象调用 enable_commands() 函数后，系统驱动（driver）还会在对象中寻找 catch_tell() 方法，如果找到，每次有消息发给对象（如通过外部函数 say()）时都会调用 catch_tell() 方法。(仅针对非玩家生物)

    从 FluffOS  3.0-alpha7 开始，本函数接受整型参数，默认值为0，作用和旧版一样，仅仅是启用命令，但不设置动作。如果参数 setup_actions > 0，驱动会调用对象所在环境及环境中所有中所有对象的 init() 方法。

### BUGS ###

    尽量不要在 create() 方法以外的地方调用 enable_commands()，否则可能会产生奇怪的问题。

### 参考 ###

    this_object(3), living(3), add_action(3), command(3), catch_tell(4), say(3), create(4)

### 翻译 ###

    雪风(i@mud.ren)
