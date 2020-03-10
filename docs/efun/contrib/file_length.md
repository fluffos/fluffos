---
layout: default
title: contrib / file_length.pre
---

### NAME

    file_length - return the line count of a file

### SYNOPSIS

    int file_length(string);

### DESCRIPTION

    returns

    - line count
    - -1 in case of error (e.g insufficient privs)
    - -2 if file is directory

### SEE ALSO

    file_size(3)
