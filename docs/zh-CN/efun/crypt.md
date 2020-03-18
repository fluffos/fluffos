---
layout: default
title: strings / crypt
---

### 名称

    crypt() - 加密一个字符串

### 语法

    string crypt( string str, string seed );

### 描述

    以字符串 `seed` 为种子加密字符串 `str`，如果 `seed` 是 0，使用随机种子。

    如果以字符串 `str` 加密后的字符串做为种子继续对字符串 `str` 加密，返回字符串还是这个种子。

    请注意，fluffos 和 mudos 的加密方式不同，如果要兼容，只能使用 oldcrypt() 外部函数加密。

### 示例

    用 crypt() 函数验证密码的示例：

    string crypt_pwd = crypt(pwd, 0);
    if (crypt(pwd, crypt_pwd) == crypt_pwd)
    {
        // ...
    }

### 参考 ###

    crypt(3)

### 翻译 ###

    雪风(i@mud.ren)
