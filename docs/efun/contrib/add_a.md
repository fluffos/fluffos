---
title: contrib / add_a
---
# add_a

### NAME

    add_a() - prefix a string with the correct indefinite article

### SYNOPSIS

    string add_a( string str );

### DESCRIPTION

    Returns `str` with "a " or "an " prepended, choosing the article
    from the following word's sound rather than just its first letter:
    vowels take "an", but the special cases "us..." ("a user", "a use"),
    "hour..." ("an hour"), and words already beginning "a "/"an " are
    handled. A string of only spaces yields "a ".

### SEE ALSO

    add_a(3), vowel(3)
