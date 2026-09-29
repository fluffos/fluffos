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

:::warning[默认启用 Unicode 属性]

模式以 `PCRE2_UCP` 编译，因此 `\d` 匹配任意 Unicode 数字、`\w` 匹配任意字母。
用于*校验*输入的模式会比 PCRE2 升级之前接受更多字符串。传入 `PCRE_NO_UCP`
（或 `([ "no_ucp": 1 ])`）可恢复仅 ASCII 的字符类 —— 详见
[pcre_match](pcre_match)。

:::

### SEE ALSO

    pcre_config(3), pcre_convert(3), pcre_match(3)
