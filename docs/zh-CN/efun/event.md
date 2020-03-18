---
layout: default
title: contrib / event.pre
---

### 名称

    event() - 呼叫指定对象中的 event 方法

### 语法

    void event(object ob, string, ...);
    void event(object *ob, string, ...);

### 描述

    这个外部函数调用指定对象中的 `event_` + string(...) 方法。如果是对象数组，每个对象都会调用。如果对象是单个对象，对象和对象中的所有对象都会调用。

### 翻译

    雪风(i@mud.ren)
