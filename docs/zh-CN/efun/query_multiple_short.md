---
title: contrib / query_multiple_short
---
# query_multiple_short

### 名称

    query_multiple_short() - 把多个对象合并为一句简短描述

### 语法

    string query_multiple_short( mixed *items, int|string|void type, int|void no_dollars, int|void quiet, int|void dark );

### 描述

    把一个由物品（对象或其 short 字符串）组成的数组格式化为一句自然语言的
    列举——将相同的 short 归并并计数，例如“两把剑、一面盾和一些硬币”。

    可选参数用于调整输出：`type` 指定向对象查询哪一种 short（某个命名类别）；
    `no_dollars` 抑制用于颜色/位置从句的 `$` 标记；`quiet` 略去被标记为
    quiet 的物品；`dark` 生成在黑暗中所见的描述。这是 Discworld mudlib 的
    物品栏辅助函数。

### 参考

    query_multiple_short(3)
