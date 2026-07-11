---
title: contrib / roulette_wheel
---
# roulette_wheel

### 名称

    roulette_wheel() - 按权重随机选取映射的键

### 语法

    mixed roulette_wheel( mapping weights );

### 描述

    给定一个值为非负整数权重的映射，按各键权重成比例的概率随机返回其中一个
    键（“轮盘赌”选择）。当映射为空或权重为负/非整数时报错。

### 参考

    random(3)
