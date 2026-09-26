---
title: pcre / pcre_assoc
---
# pcre_assoc

### NAME

    pcre_assoc() - A regular pattern substring extractor

### SYNOPSIS

    mixed *pcre_assoc(string, string *, mixed *, mixed | void, void | int | mapping pcre_flags);

### DESCRIPTION

    analog with reg_assoc efun for backwards compatibility reasons but utilizing the PCRE library.

    可选参数 `pcre_flags` 用于设置 PCRE 选项（如 `PCRE_I` 大小写不敏感，`PCRE_M` 多行等），默认 0。

:::warning[默认启用 Unicode 属性]

模式以 `PCRE2_UCP` 编译，因此 `\d` 匹配任意 Unicode 数字、`\w` 匹配任意字母。
用于*校验*输入的模式会比 PCRE2 升级之前接受更多字符串。传入 `PCRE_NO_UCP`
（或 `([ "no_ucp": 1 ])`）可恢复仅 ASCII 的字符类 —— 详见
[pcre_match](pcre_match)。

:::

### SEE ALSO

    reg_assoc(3)
