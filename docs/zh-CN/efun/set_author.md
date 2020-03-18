---
layout: default
title: mudlib / set_author
---

### 名称

    set_author - 设置与对象关联的创造者（author）

### 语法

    void set_author( string author );

### 描述

    每个对象都有一个作者和域与之关联用来统计相关信息。域只能通过主控对象中的 domain_file() 方法设置，但是创造者不同，既能通过主控对象中的 author_file() 初始化，也能使用 set_author() 外部函数修改。

    set_author() 外部函数修改调用它的对象的创造者。作者可以获取对象所有影响 mudlib 统计的行为。Note that this may cause some weird numbers to occur in the categories "objects" and "array_size", since the object may have initialized arrays or been created under the original author's credit, but it could be destructed or free the arrays that it's using under another author, thus reducing numbers from a count that didn't have those same  numbers added previously. 要记住这一点，请仅在 create() 方法中使用 set_author()，并尽可能在分配任何数组之前使用它。

### 参考

    author_file(4), domain_file(4), author_stats(3), set_author(3), domain_stats(3)

### 翻译

    雪风(i@mud.ren)
