---
layout: default
title: master / valid_hide
---

### 名称

    valid_hide - 允许或不允许对象隐藏和查看隐藏的对象

### 语法

    int valid_hide( object ob );

### 描述

    在主控对象（master.c）中增加 valid_hide() 方法控制是否允许对象隐藏自己或查看其它已经隐藏的对象。当一个对象尝试使用 set_hide() 外部函数隐藏自己时，这个对象会作为唯一的参数调用 valid_hide() 方法，如果允许方法会返回 1 ，禁止则返回 0 。当需要检测一个对象是否可以查看隐藏对象时也会以相同的方式调用 valid_hide()。

### 参考

    set_hide(3)

### 翻译 ###

    雪风(i@mud.ren)
