---
layout: default
title: object / reset
---

### 名称

    reset - 让对象做自我维护

### 语法

    void reset( void );

### 描述

    在每次重置间隔之后，所有对象会调用自己的 reset() 方法（如果有的话）。如果编译驱动时定义了 LAZY_RESETS ，只有玩家附加的对象才会呼叫 reset() 方法。 reset() 方法常用来重置游戏中的对象（宝物、怪物、机关等）。

    默认重置间隔时间（rest_time）在运行时配置文件中设置，实际每个对象的重置间隔不完全一样，具体为：

    reset_time = current_time + reset_time / 2 + random(reset_time / 2)

### 参考

    set_reset(3), create(4)

### 翻译

    雪风(i@mud.ren)
