---
layout: default
title: objects / set_heart_beat
---

### 名称

    set_heart_beat() - 开启或关闭一个对象的心跳

### 语法

    int set_heart_beat( int flag );

### 描述

    参数 `flag` 为 0 时关闭对象心跳，为 1 时对象每一次心跳都会呼叫对象的 heart_beat() 方法一次（心跳频率可以在运行时配置文件中设置，默认为 1000 毫秒，即 1 秒）。参数数值大于 1 时代表每隔多少次心跳呼叫 heart_beat() 方法一次。

### 参考

    heart_beat(4), query_heart_beat(3)

### 翻译

    雪风(i@mud.ren)
