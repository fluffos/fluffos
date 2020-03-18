---
layout: default
title: contrib / get_garbage.pre
---

### 名称

    get_garbage() - 返回游戏中加载中的垃圾对象

### 语法

    object *get_garbage();

### 描述

    返回游戏中复制的垃圾对象数组（最大值为 `MAX_ARRAY_SIZE`），垃圾对象是指既没有环境没有内容物，而且也没有映射其它对象 and haven't object->super set。

### 翻译 ###

    雪风(i@mud.ren)
