---
layout: default
title: contrib / store_class_member.pre
---

### 名称

    store_class_member

### 语法

    mixed store_class_member(mixed instantiated_class, int class_element, mixed value);

### 描述

    保存结构体 `instantiated_class` 的第 `class_element` 个成员的值为 `value`，返回更新后的结构体。

### 示例

    class person {
        string name ;
        int age ;
        string city ;
    }

    void fun()
    {
        class person me = new(class person) ;

        me->name = "Foo" ;
        me->age = 42 ;
        me->city = "Fooville" ;

        write( sprintf("%O\n", me) ) ;

        // Result:
        // CLASS( 3 elements
        //   "Foo",
        //   42,
        //   "Fooville"
        //  )

        me = store_class_member( me, 1, 43 ) ;

        write( sprintf("%O\n", me) ) ;

        // Result:
        // CLASS( 3 elements
        //   "Foo",
        //   43,
        //   "Fooville"
        //  )
    }

### 参考

    fetch_class_member(3)

### 翻译

    雪风(i@mud.ren)
