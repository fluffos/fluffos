---
layout: doc
title: pcre / pcre_assoc
---
# pcre_assoc

### NAME

    pcre_assoc() - A regular pattern substring extractor

### SYNOPSIS

    mixed *pcre_assoc(string, string *, mixed *, mixed | void, void | int pcre_flags);

### DESCRIPTION

    analog with reg_assoc efun for backwards compatibility reasons but utilizing the PCRE library.

    可选参数 `pcre_flags` 用于设置 PCRE 选项（如 `PCRE_I` 大小写不敏感，`PCRE_M` 多行等），默认 0。

### SEE ALSO

    reg_assoc(3)
