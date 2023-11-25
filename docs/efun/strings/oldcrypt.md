---
layout: doc
title: strings / oldcrypt
---
# oldcrypt

### NAME

    oldcrypt() - encrypt a string

### SYNOPSIS

    string oldcrypt( string str, string seed );

### DESCRIPTION

    Encrypt the string `str` using the first two characters from string 
    `seed`. If `seed` is zero, then a random seed will be used.

    The encrypted result can be used as the `seed` of a second encrypt,
    which will return the original encrypted result if both `str` inputs
    were identical.

### NOTE

    The encryption methods of FluffOS and MudOS are different. For MudOS
    compatibity you should use oldcrypt().

### EXAMPLE

    Example of verifying a password using oldcrypt():

    // turn user input into crypted password
    string cryptPasswd = oldcrypt(input, 0);

    // store crypted password in user save somewhere

    // compare login input to stored crypted password
    if (oldcrypt(input, cryptPasswd) == cryptPasswd) {
        // valid password
    } else {
        // invalid password attempt
    }

### SEE ALSO

    crypt(3), hash(3)