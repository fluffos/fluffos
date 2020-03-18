---
layout: default
title: master / get_root_uid
---

### 名称

    get_root_uid - 取得根用户ID(root uid)

### 语法

    string get_root_uid( void );

### 描述

    在驱动程序编译时定义了 PACKAGE_UIDS 后有效。

    每当驱动程序加载主控对象(master object)时会调用这个方法，验证主控对象是否已经加载，并取得 MUD 定义的根用户ID。这个方法应该返回一个字符串，如："ROOT"。

### 参考

    get_bb_uid(4)

### 翻译

    雪风(i@mud.ren)
