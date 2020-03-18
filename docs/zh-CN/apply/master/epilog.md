---
layout: default
title: master / epilog
---

### 名称

    epilog - 以数组形式返回准备预加载的对象的文件名列表

### 语法

    string *epilog( int load_empty );
    void preload( string filename );

### 描述

    在主控对象载入后驱动程序调用主控对象中的 epilog() 方法。MUDLIB 通常使用 epilog() 方法初始化主控对象中的数据结构。epilog() 方法应该返回一个包含 mudlib 想要预加载的对象的文件名数组，也就是说，在第一个玩家登录前完成加载。驱动程序会把返回的数组中的每个文件名作为参数调用主控对象中的 preload(filename) 方法。

    在 MUDOS 中指明参数 -e 启动驱动程序时，参数 `load_empty` 是非零值，但是这个 -e 参数在 FLUFFOS 中已经无效。

### 参考

    preload(4)

### 翻译

    雪风(i@mud.ren)
