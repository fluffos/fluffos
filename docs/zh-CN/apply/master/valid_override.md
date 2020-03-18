---
layout: default
title: master / valid_override
---

### 名称

    valid_override - 控制 `efun::` 前缀的使用

### 语法

    int valid_override( string file, string efun_name , string main_file );

### 描述

    在主控对象中增加 valid_override() 方法以控制 `efun::` 前缀的使用。每当驱动程序试图编译一个以 `efun::` 为前缀调用外部函数的文件时都会呼叫主控对象中的 valid_override() 方法。如果 valid_override() 返回 0 ，编译会失败。因此 valid_override() 提供了一种方式保证修改过功能的外部函数（同名模拟外部函数）无法使用 efun:: 绕过。

    参数 `file` 是实际使用 efun:: 的文件，参数 `efun_name` 是通过 efun:: 调用的外部函数名称，参数 `main_file` 是准备编译的文件（包含扩展名 `.c`，而且通过使用 #include，`file` 和 `main_file` 可以不同）。

    这里是一个通过 valid_override 限制重写外部函数的示例：

    int valid_override(string file, string name, string main_file)
    {
        if (file == "/adm/obj/simul_efun")
        {
            return 1;
        }
        if (name == "destruct")
            return 0;
        if (name == "shutdown")
            return 0;
        if (name == "snoop")
            return 0;
        if (name == "exec")
            return 0;
        return 1;
    }

### 作者

    Truilkan@Basis

### 参考

    valid_object(4), function_exists(3)

### 翻译 ###

    雪风(i@mud.ren)
