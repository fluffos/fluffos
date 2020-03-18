---
layout: default
title: mudlib / set_light
---

### 名称

    set_light() - 更新或查询对象的光亮级别

### 语法

    int set_light( int light_level_adjustment );

### 描述


    参数 <light_level_adjustment> 为 0 为查询对象的当前光亮级别，为正代表增加当前对象的光亮级别，为负代表减少当前对象的光亮级别。

    译者注：原则上这个函数不应该是驱动该提供的，因为这是很容易被模拟出来的游戏功能，但是既然驱动提供了，那就优先用驱动提供的吧。

### 翻译

    雪风(i@mud.ren)
