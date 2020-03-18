---
layout: default
title: object / move_or_destruct
---

### 名称

    move_or_destruct - 询问对象是否需要移动位置

### 语法

    int move_or_destruct( void );

### 描述

    如果一个对象被销毁，在销毁前其内容对象会呼叫这个 apply 方法。如果呼叫 move_or_destruct() 方法的对象不把自己移到即将被销毁的对象外，也会被一起销毁。

### 参考

    destruct(3), move_object(3), init(4)

### 翻译

    雪风(i@mud.ren)
