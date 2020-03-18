---
layout: default
title: interactive / exec
---

### 名称

    exec() - 把连线玩家从一个对象切换到另一个对象

### 语法

    int exec( object to, object from );

### 描述

    这个外部函数允许把玩家连接从一个对象迁移到另一个对象。也就是说，在成功执行 exec(to, from) 后，interactive(to) 会返回 1，interactive(from) 会返回 0，控制 `from` 的玩家改为控制 `to`。

    请注意，这个外部函数的功能过于强大，为保证游戏安全，请谨慎使用。较为适合的限制使用 exec() 的方式是定义一个同名模拟外部函数(simul_efun)，并使用 valid_override() apply 方法来限制这个模拟外部函数的重写（efun::exec()）。

    如果切换成功 exec() 会返回 1，否则返回 0。

### 参考

    interactive(3), valid_override(4)

### 翻译

    雪风(i@mud.ren)
