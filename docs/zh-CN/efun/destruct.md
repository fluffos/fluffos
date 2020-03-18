---
layout: default
title: objects / destruct
---

### 名称

    destruct() - 从游戏中移除一个对象

### 语法

    void destruct( object ob );

### 描述

    从游戏中完全销毁并移除对象 `ob`，如果不指定参数 `ob`，默认为 this_object()。在呼叫 destruct() 后，如果对象是 this_object()，this_object() 的代码仍然会继续执行，但最好马上返回一个值，而且所有指向被销毁的对象的变量值都会变成 0。

    即将被销毁的对象的所有内容对象都会呼叫自己的 move_or_destruct() 方法，如果呼叫 move_or_destruct() 方法的对象不把自己移到即将被销毁的对象外，也会被一起销毁。

### 参考

    clone_object(3), new(3), move_or_destruct(4)

### 翻译

    雪风(i@mud.ren)
