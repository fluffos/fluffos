---
layout: default
title: interactive / ed
---

### 名称

    ed() - 编辑一个文件

### 语法

    void ed( string file, string exit_fn, int restricted );
    void ed( string file, string write_fn, string exit_fn, int restricted );

### 描述

    这个外部函数仅在驱动编译时定义了 __OLD_ED__ 后有效。

    这是一个很有趣的函数，它会打开内置编辑器来编辑指定文件 `file`。这个编辑器基本和 UNIX ed 兼容，在编辑器中可以输入 `h` 查看使用帮助。

    The <write_fn> function allows the mudlib to handle file locks and administrative logging of files modified. When the editor writes to a file, the driver will callback the <write_fn> function twice. The first time, the function is called before the write takes place -- <flag> will be 0. If the function returns TRUE, the write will continue, otherwise it will abort. The second time, the function is called after the write has completed -- <flag> will be non-zero. This callback function should have the form:

    int write_fn(string fname, int flag)

    When the editor is exited, the driver will callback the <exit_fn> function. This function allows the mudlib to clean up. This callback function has the form:

    void exit_fn()

    The optional <restricted> flag limits the editor's capabilities, such as inserting a file, and saving using an alternate file name.

### 参考

    regexp(3), valid_read(4), valid_write(4), get_save_file_name(4)

### 翻译 ###

    雪风(i@mud.ren)
