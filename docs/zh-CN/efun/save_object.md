---
layout: doc
title: objects / save_object
---
# save_object

### 名称

    save_object() - 保存当前对象的变量值到一个文件

### 语法

    int save_object( string name, int flag );
    string save_object( int flag );

### 描述

    保存当前对象中所有非 nosave 类型的全局变量到文件 `name` 中。主控对象中的 valid_write() 方法检测是否允许写文件。第二个参数为可选参数，是一个位域（00、01、10、11），如果 0 位是 1（如：01、11），值为 0 的变量也会保存（正常情况下不会保存），对象类型变量一只保存为 0，如果 1 位是 1（如：10、11），存档文件会被压缩。

    如果不提供文件名（省略该参数，或只传入整型的标志位域），序列化后的数据不会写入磁盘，而是作为字符串返回，可配合 restore_object() 的字符串形式使用。

### 返回值

    带文件名时，存档成功返回 1，失败返回 0；不带文件名的形式则返回存档数据字符串。

### 参考

    restore_object(3)

### 翻译

    雪风(i@mud.ren)
