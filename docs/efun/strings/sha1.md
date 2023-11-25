---
layout: doc
title: strings / sha1
---
# sha1

### NAME

    sha1() - hash a string using SHA-1

### SYNOPSIS

    string sha1(string str | buffer bf);

### DESCRIPTION

    Returns the SHA-1 hash of string `str` or buffer `bf` using SHA-1.

    This requires PACKAGE_CRYPTO to be enabled when compiling the driver.

    Note:
    The `hash(algo, str)` external function can handle SHA-1 and more.

### EXAMPLE

    sha1("something") = "1af17e73721dbe0c40011b82ed4bb1a7dbe3ce29"

### SEE ALSO

    hash(3)