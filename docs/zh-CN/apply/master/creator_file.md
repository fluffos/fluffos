---
layout: default
title: master / creator_file
---

### 名称

    creator_file - 给最新创建的对象指定用户ID(uid)

### 语法

    string creator_file(string filename);

### 描述

    在驱动程序编译时定义了 PACKAGE_UIDS 后有效。

    每当一个新对象创建时，驱动程序会调用主控对象中的这个方法，新对象的文件名 `filename` 做为唯一的参数传递给 creator_file() 方法，方法的返回值会设置为新对象的用户ID(uid)，如果驱动程序编译时定义了 AUTO_SETEUID ，这个值也会设置为对象的有效用户ID(euid)。

    一个例外：如果驱动程序编译时开启了 AUTO_TRUST_BACKBONE 选项，并且 creator_file() 方法返回骨干用户ID(backbone uid，通过主控对象中的 get_bb_uid() 方法指定)，那么新对象的 uid 和 euid 就和载入它的对象相同。

### 参考

    seteuid(3), new(3), clone_object(3), call_other(3)

### 翻译

    雪风(i@mud.ren)
