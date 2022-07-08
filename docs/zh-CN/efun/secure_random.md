---
layout: default
title: numbers / secure_random
---

### 名称

    secure_random() - 返回一个不可预测的伪随机数，但速度比random()慢

### 语法

    int secure_random( int n );

### 描述

    从范围[0 .. (n -1)] (包含)中返回加密安全的随机数。

    On Linux & OSX, this function explicitly use randomness from /dev/urandom, on windows it is implementation defined.

### 参考

    random()
