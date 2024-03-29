---
layout: doc
title: interactive / printf
---
# sprintf

### 名称

    sprintf - 返回格式化的字符串

### 语法

    string sprintf(string format, ... );

### 描述

    格式化输出信息给当前玩家，format 完整形式为：%[flag][width][.precision]type

    当前版本支持以下格式控制符，其中标志位控制符可组合使用:

#### 标志位（flag）
    " "     用于整数或者小数，输出值为正时冠以空格，为负时冠以负号。

    "+"     用于整数或者小数，表示输出符号（正负号）。如果没有，那么只有负数才会输出符号。

    "-"     示左对齐。如果没有，就按照默认的对齐方式，默认为右对齐。

    "|"     对齐栏位中心。

    "="     用于字符串，如果字符串长度超出栏位宽度，使用纵列模式(column mode)，字符串会自动换行。

    "#"     用于字符串， 在柆位内使用表格模式(table mode)换行（'\n'）输出单词列表。

    "0"     必须紧随宽度整数前面，将输出的资料空栏位用0填充，直到占满指定栏位列宽为止。

    "'X'"   用单引号中的字符填充栏位（默认为用空格填充）(如果即指定用0填充，又指定用字符填充，以后
            指定的为准。注意：如果要用“'”、“"”或“\”填充，必须使用转义字符“\'”、“\"”、“\\”。

    "@"     参数是数组，除 "@" 外的其它格式信息会应用于数组中的每一个元素。

#### 输出最小宽度（width）
    整数    指定栏位最小宽度（列宽）， 使用 '*' 则在输出参数列表中指定宽度。若实际位数多于指定的宽
            度，则按实际位数输出，若实际位数少于定义的宽度则补以空格或0。

#### 精度（.precision）
    ".整数"     指定栏位的精度, 普通字符串在此之后会被截断(如果精度超出列宽，则列宽等于精度),
                表格模式使用精度指定列数(如果未指定，表格会自动计算最适合的结果)。

    ":整数"     指定栏位宽度和精度。如果整数前补0，则代替空格使用0补满栏位。

#### 类型（type）

    %       简单的说，就是输出 % 用，示例：printf("%%");

    O       参数格式为 LPC 数据类型。（可以输出任何类型，方便DEBUG）

    s       参数格式为字符串类型。

    d, i    把整型参数输出为有符号十进制整数。（i为旧式写法）

    c       把整型参数输出为字符。

    o       把整型参数输出为无符号八进制整数。（不输出前缀0）

    x       把整型参数输出为无符号十六进制整数。（不输出前缀0x，小写a-f）

    X       把整型参数输出为无符号十六进制整数。（不输出前缀0x，大写A-F）

    f       参数格式为浮点数。

### 返回值

    sprintf() 返回格式化的字符串。

### 作者

    Sean A. Reith (Lynscar)

### 参考

    sscanf(3)

### 翻译

    雪风(i@mud.ren)
