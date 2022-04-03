---
layout: default
title: contrib / assemble_class.pre
---

### 名称

    assemble_class

### 语法

    mixed assemble_class( mixed *elements );

### 描述

    用数组 `elements` 的元素实例化一个结构体。

### 示例

    mixed *elements = ({ "Foo", 42, "Fooville" }) ;
    mixed cl = assemble_class( elements ) ;

    write( sprintf( "%O\n", cl ) ) ;

    // CLASS( 3 elements
    //   "Foo",
    //   42,
    //   "Fooville"
    //  )

### 警告

    If you have a previously defined class and assign the result from
    assemble_class to a variable, the variable will be shaped from the
    result of assemble_class. No error is caught by the shapes being
    different, and the original specification is ignored.

    class person {
        string name ;
        int age ;
        string city ;
    }

    void fun()
    {
        class person me ;

        me = assemble_class( ({ "Would", "You", "Like", 42, "Bubblegums" }) ) ;
        write( sprintf( "%O\n", me ) ) ;

        // CLASS( 5 elements
        //   "Would",
        //   "You",
        //   "Like",
        //   42,
        //   "Bubblegums"
        //  )
    }

### 参考

    disassemble_class(3), fetch_class_member(3)
