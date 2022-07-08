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

    第二种用法用于定义一个结构体类型（struct | class）的变量。用法为：

        class <class_name> <var_name> = new(class <class_name>);
        class <class_name> <var_name> = new(class <class_name>, <var>:<value>, ...)

    示例如下：

    class example {
        string name;
        int age;
    }

    void create()
    {
        // 变量
        class example instance = new(class example);
        // 赋值
        instance->age = 18;
        instance->name = "fluffos";

        // 或直接在声明时初始化
        instance = new(class example, name:"fluffos", age:18);
    }

### 参考 ###

    clone_object(3), destruct(3), move_object(3))

### 翻译 ###

    雪风(i@mud.ren)
