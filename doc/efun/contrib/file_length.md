---
layout: default
title: contrib / file_length.pre
---

int file_length(string);

returns

- line count
- -1 in case of error (e.g insufficient privs)
- -2 if file is directory

see file_size(3)
