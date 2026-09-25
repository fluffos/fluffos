---
title: pcre / pcre_extract
---
# pcre_extract

### NAME

    pcre_extract() - extract matching parts

### SYNOPSIS

    string *pcre_extract(string, string, void|int|mapping include_named, void|int|mapping pcre_flags);

### DESCRIPTION

    返回模式中捕获分组组成的数组。

    第三个参数 `include_named` 可选，默认 0。当其非零时，返回值末尾总会追加一个映射，键为分组名称，值为对应的捕获内容。若正则没有命名分组（或均未匹配），映射为空；否则仅包含实际参与匹配的分组。

    第四个参数 `pcre_flags` 可选，默认 0，用于设置 PCRE 选项（如 `PCRE_I` 大小写不敏感，`PCRE_M` 多行等）。

    示例（命名分组）：

        ({"alpha", "99", (["word": "alpha", "num": "99"])})

:::warning[默认启用 Unicode 属性]

模式以 `PCRE2_UCP` 编译，因此 `\d` 匹配任意 Unicode 数字、`\w` 匹配任意字母。
用于*校验*输入的模式会比 PCRE2 升级之前接受更多字符串。传入 `PCRE_NO_UCP`
（或 `([ "no_ucp": 1 ])`）可恢复仅 ASCII 的字符类 —— 详见
[pcre_match](pcre_match)。

:::

### SEE ALSO

    pcre_version(3), pcre_replace(3)
