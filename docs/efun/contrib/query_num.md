---
layout: doc
title: contrib / query_num.pre
---
# query_num

### NAME

    query_num

### SYNOPSIS

    string query_num(int num, int many: 0);

### DESCRIPTION

    Warning: English only!!!

    Converts `num` into a string representation. If `many` is greater than 0
    and `num` is greater than `many`, the resulting string is "many".

    Any `num` greater than 99,999 is always "many", the same for any `num` less
    than 0.

### EXAMPLES
    query_num(99999) ;
    "ninety-nine thousand, nine hundred and ninety-nine"

    query_num(99999, 50) ;
    "many"

    query_num(0)
    "zero"

    query_num(-10)
    "many"

### WORKAROUND

    To handle negative numbers, you can override the function in your own
    simul_efun object as follows:
```c
varargs string query_num(int x, int many) {
    string sign;

    if(nullp(x))
        error("Too few arguments to 'query_num'.") ;

    if(!intp(x))
        error("Bad argument 1 to 'query_num'.") ;

    sign = x < 0 ? "negative " : "" ;
    x = abs(x) ;

    return sign + efun::query_num(x, many) ;
}
````

### SEE ALSO

    pluralize(3)
