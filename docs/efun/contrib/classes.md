---
layout: default
title: contrib / classes.pre
---

### NAME

    classes() - return names of classes used by object

### SYNOPSIS

    mixed *classes(object ob, int verbose);

### DESCRIPTION

    Return a string array consisting of names of classes used by <ob>.

    The <verbose> parameter is optional and by default 0.

    If <verbose> is non-zero, then  additional  information  will  be
    returned.
    
    
### EXAMPLE

    string *classes_used = classes( ob ) ; 
    // ({ "class_name", }) ;

    mixed *classes_used = classes( ob, 1 ) ;

    // ({
    //     ({
    //         "class_name",
    //         ({
    //             "member_name",
    //             "type"
    //         }),
    //         ...
    //     }),
    //     ...
    // })

### SEE ALSO

    num_classes(3)
