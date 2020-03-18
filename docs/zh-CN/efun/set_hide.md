---
layout: default
title: objects / set_hide
---

### 名称

    set_hide() - 对可隐藏对象设置隐藏标志(flag)

### 语法

    void set_hide( int flag );

### 描述

    为当前对象设置隐藏标志 `flag`，0 代表不隐藏，1 代表隐藏。当对象隐藏后，只有其它有隐藏权限（主控对象中的 valid_hide() 判断）的对象可以使用 find_object()、users()、children()、all_inventory()等查找此对象，objects()不会显示隐藏对象。

### 参考

    valid_hide(4)

### 翻译

    雪风(i@mud.ren)
