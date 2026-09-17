---
title: pcre / pcre_info
---
# pcre_info

### NAME

    pcre_info() - 已编译模式的元数据

### SYNOPSIS

    mapping pcre_info(string pattern, void|int|mapping pcre_flags);

### DESCRIPTION

    按其它 `pcre_*` efun 的默认值和选项编译 `pattern`，返回捕获组数、
    命名组、最短匹配长度、是否 JIT 等。详见英文页。Callout、DFA、
    序列化字节码不暴露给 LPC。

### SEE ALSO

    pcre_config(3), pcre_convert(3), pcre_match(3)
