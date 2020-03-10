---
layout: default
title: contrib / classes.pre
---

### NAME

    classes() - return names of classes used by object

### SYNOPSIS

    mixed *classes(object, int default : 0);

### DESCRIPTION

    return array consisting of names of classes used by object.
    if int is non-zero, then  additional  information  will  be
    written, the returned array has a form like this:
    ({
        ({
            "class_name",
            ({
                "member_name",
                "type"
            }),
            ...
        }),
        ...
    })

### SEE ALSO

    num_classes(3)
