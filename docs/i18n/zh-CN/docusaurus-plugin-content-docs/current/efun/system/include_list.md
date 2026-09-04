---
title: system / include_list
---
# include_list

### 名称

    include_list() - 获取对象编译时实际 #include 的文件列表

### 语法

    string *include_list( object obj );

### 描述

    返回对象编译时实际 `#include` 的文件名数组（含嵌套 include，按首次出现顺序，重复项只保留一次）。对象自身的源文件不在列表中。每个文件名带前导斜杠。

    若编译器打开了配置中的全局头文件，也会列入。处于假 `#if` 分支中的 `#include` 不会出现：那个文件并未被打开。

    这是 `inherit_list()` 对应的 include 侧接口。只重编译变更对象的 mudlib 可以遍历 `include_list(ob)`，判断哪些头文件变化会迫使 `ob` 重新编译。

    若未提供对象，默认使用 this_object()。

### 参考

    inherit_list(3), deep_inherit_list(3), recompile_object(3)
