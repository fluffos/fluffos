---
title: pcre / pcre_replace_callback
---
# pcre_replace_callback

### NAME

    pcre_replace_callback() - string replace uses a callback to get the replace string

### SYNOPSIS

    string pcre_replace_callback(string, string, string | function, ..., void|int|mapping pcre_flags);

### DESCRIPTION

    returns a string where all captured groups have been replaced by the
    return value of function pointe fun or function fun in object ob.
    (called with the matched string and match number, starting with 0)

    可选参数 `pcre_flags` 用于设置 PCRE 选项（如 `PCRE_I` 大小写不敏感，
    `PCRE_M` 多行等），默认 0。

:::warning[默认启用 Unicode 属性]

模式以 `PCRE2_UCP` 编译，因此 `\d` 匹配任意 Unicode 数字、`\w` 匹配任意字母。
用于*校验*输入的模式会比 PCRE2 升级之前接受更多字符串。传入 `PCRE_NO_UCP`
（或 `([ "no_ucp": 1 ])`）可恢复仅 ASCII 的字符类 —— 详见
[pcre_match](pcre_match)。

:::

### SEE ALSO

    pcre_assoc(3), pcre_cache(3), pcre_extract(3), pcre_replace(3)
