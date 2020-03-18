---
layout: default
title: objects / reload_object
---

### 名称

    reload_object - 把对象还原到刚载入时的状态

### 语法

    void reload_object( object ob );

### 描述

    当对象 `ob` 执行 reload_object()后，所有由驱动程序设置的属性全部初始化（heart_beat，call_outs，light，shadow等），所有变量也初始化，并且调用 create() 方法。这和销毁并重载对象等效果类似，但是，不会存取磁盘数据和执行语法分析，如果代码修改过，reload_object 不会载入新代码。

### 参考

    export_uid(3), new(3), clone_object(3), destruct(3)

### 翻译

    雪风(i@mud.ren)
