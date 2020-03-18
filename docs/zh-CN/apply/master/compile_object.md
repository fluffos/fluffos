---
layout: default
title: master / compile_object
---

### 名称

    compile_object  - 做为 mudlib 中虚拟对象的接口

### 语法

    object compile_object( string pathname );

### 描述

    当 mudlib 需要驱动程序载入一个不存在的对象时驱动程序会调用主控对象中的 compile_object() 方法。如：当 mudlib 调用 call_other("/obj/file.r", "some_function") 或 new("/obj/file.r") 且文件 "/obj/file.r.c" 不存在时驱动程序会调用主控对象中的 compile_object("/obj/file.r") 方法。如果 mudlib 不想关联对象到文件名 "/obj/file.r"，compile_object() 方法将返回 0 ，如果 mudlib 想关联一个对象到文件名 "/obj/file.r" ，compile_object() 应该返回被关联的对象。当把对象和文件名关联后，就像这个文件真的存在一样，而通过文件名载入的对象就是 compile_object() 方法返回的对象。

### 翻译

    雪风(i@mud.ren)
