---
layout: default
title: interactive / write
---

### 名称

    write() - 对当前玩家送出信息

### 语法

    void write(mixed str);

### 描述

    对当前玩家显示 'str' 信息。 'str' 如果是数字会自动转换成字符串。对象类型也可以正常显示，但其它类型比如数组、映射、函数指针或缓冲区类型会显示为<ARRAY>、<MAPPING>、<FUNCTION>和<BUFFER>。

### 参考

    message(3), tell_object(3), tell_room(3), shout(3), say(3)

### 翻译

    雪风(i@mud.ren)
