---
layout: default
title: objects / save_object
---

### 名称

    save_object() - 保存当前对象的变量值到一个文件

### 语法

    int save_object( string name, int flag );

### 描述

    保存当前对象中所有非 nosave 类型的全局变量到文件 `name` 中。主控对象中的 valid_write() 方法检测是否允许写文件。第二个参数为可选参数，是一个位域（00、01、10、11），如果 0 位是 1（如：01、11），值为 0 的变量也会保存（正常情况下不会保存），对象类型变量一只保存为 0，如果 1 位是 1（如：10、11），存档文件会被压缩。

### 返回值

    存档成功返回 1，失败返回 0。

### 参考

    restore_object(3)

### 翻译

    雪风(i@mud.ren)
