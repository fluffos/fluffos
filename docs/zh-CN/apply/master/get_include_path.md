---
layout: default
title: master / get_include_path
---

### 名称

    get_include_path - 确定一个对象的头文件所在目录

### 语法

    mixed get_include_path( string object_path );

### 描述

    当驱动程序开始编译一个新对象时会调用这个方法确定从哪个目录寻找头文件。和运行时配置文件中选项 `include directories` 固定头文件目录相反，这个方法可以基于存储位置动态指定头文件目录。

    参数 `object_path` 是将被编译的对象的绝对路径，返回值是一个字符串数组，数组元素或者是期望的头文件包含目录的绝对路径，或者是代表驱动程序运行时配置文件中设置的头文件包含目录的特殊字符串 ":DEFAULT:"。

### 示例

    string *get_include_path(string file)
    {
        string *t = explode(file, "/");
        if(t[1] == "Domain")    // Domains have their own include directory
            return ({ "/Domain/" + t[2] + "/include", ":DEFAULT:" });
        else                    // use default for others
            return ({ ":DEFAULT:" });
    }

### 翻译

    雪风(i@mud.ren)
