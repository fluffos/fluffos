---
layout: default
title: objects / new
---

### 名称 ###

    new() - 加载一个对象的复制

### 语法 ###

    object x = new( string name, ... );
    class foo x = new( class foo );

### 描述 ###

    第一种用法和 clone_object() 完全一样，new() 相当于 clone_object() 的别名。

    第二种用法用于定义一个结构体类型（struct | class）的变量。示例如下：

    class example {
        string name;
        int age;
        string *lover;
    }

    void create()
    {
        // 变量
        class example instance = new(class example);
        // 赋值
        instance->age = 24;
        instance->name = "Ivy";
        instance->lover = ({"Alice", "Vivian", "lucy"});
        // ...
    }

### 参考 ###

    clone_object(3), destruct(3), move_object(3))

### 翻译 ###

    雪风(i@mud.ren)
