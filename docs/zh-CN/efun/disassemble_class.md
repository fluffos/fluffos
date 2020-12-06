---
layout: default
title: contrib / disassemble_class.pre
---

### 名称

    disassemble_class

### 语法

    mixed *disassemble_class( mixed instantiated_class );

### 描述

    返回一个包含已实例化的结构体 `instantiated_class` 所有成员的值的 mixed 类型的数组。

### 示例

    class person {
        string name ;
        int age ;
        string city ;
    }

    void fun()
    {
        mixed *result ;
        class person me = new(class person);

        me->name = "Foo" ;
        me->age = 42 ;
        me->city = "Fooville" ;

        result = disassemble_class( me ) ;

        write( sprintf("%O\n", result ) ) ;

        // ({ /* sizeof() == 3 */
        //   "Foo",
        //   42,
        //   "Fooville"
        // })
    }

### 参考

    assemble_class(3)

### 翻译

    雪风(i@mud.ren)
