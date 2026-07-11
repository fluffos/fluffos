---
title: contrib / replace_objects
---
# replace_objects

### 名称

    replace_objects() - 递归地把值中的对象渲染为字符串

### 语法

    mixed replace_objects( mixed value );

### 描述

    遍历 `value`（对象、数组、类或映射，任意深度），返回一个副本，其中每个
    对象都被替换为一段描述性字符串——其对象名，外加由主控对象 object_name()
    应用得到的可读标签，已销毁的对象则标为 "(destructed)"。数组、类、映射
    保持结构，其它值原样保留。主要用作调试/转储辅助。

### 参考

    sprintf(3)
