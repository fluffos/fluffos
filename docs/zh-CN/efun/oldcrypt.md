---
layout: default
title: strings / crypt
---

### 名称 ###

    oldcrypt() - 加密一个字符串

### 语法 ###

    string oldcrypt( string str, string seed );

### 描述 ###

    以字符串 `seed` 为种子加密字符串 `str`，如果 `seed` 是 0，使用随机种子。

    如果以字符串 `str` 加密后的字符串做为种子继续对字符串 `str` 加密，返回字符串还是这个种子。

### 示例 ###

    用 oldcrypt() 函数验证密码的示例：

    string crypt_pwd = oldcrypt(pwd, 0);
    if (oldcrypt(pwd, crypt_pwd) == crypt_pwd)
    {
        // ...
    }

### 参考 ###

    crypt(3)

### 翻译 ###

    雪风(i@mud.ren)
