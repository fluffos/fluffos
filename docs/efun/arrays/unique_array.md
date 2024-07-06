---
layout: doc
title: arrays / unique_array
---
# unique_array

### NAME

    unique_array() - partitions an array of objects into groups

### SYNOPSIS

    mixed unique_array( mixed *arr, function f | string fun, void | mixed skip );

### DESCRIPTION

    Returns an array of arrays containing similar elements grouped together,
    based upon the results of a function call mapped to each element.

    If an array of objects is passed in as `arr` and string `fun` is also
    passed, `fun` will be called upon each object in the array. Otherwise,
    function `f` will receive each element as a function parameter.

    The optional `skip` parameter enables a pre-filter on `arr`, skipping
    elements which match `skip`.

### RETURN VALUE

    The return value is an array of arrays of objects on the form:

    ({
        ({Same1:1, Same1:2, Same1:3, .... Same1:N }),
        ({Same2:1, Same2:2, Same2:3, .... Same2:N }),
        ({Same3:1, Same3:2, Same3:3, .... Same3:N }),
                    ....
        ({SameM:1, SameM:2, SameM:3, .... SameM:N }),
    })

### EXAMPLE

`mixed *` style:
```c
unique_array(({ "A", "B", "B", "C", "C", "C" }), (: $1 :));
// ({
//     ({ "A" }),
//     ({ "B", "B" }),
//     ({ "C", "C", "C" })
// })

unique_array(({ 1, 2, 2, 3, 3, 3 }), (: $1 :));
// ({
//     ({ 1 }),
//     ({ 2, 2 }),
//     ({ 3, 3, 3 })
// })
```

`object *` style:
```c
unique_array(({
    OBJ(branch /obj/branch#1),
    OBJ(tree /obj/tree#2),
    OBJ(tree /obj/tree#3)
}), "query_name");
// ({
//     ({ OBJ(branch /obj/branch#1) }),
//     ({ OBJ(tree /obj/tree#2), OBJ(tree /obj/tree#3) })
// })

unique_array(({
    OBJ(branch /obj/branch#1),
    OBJ(tree /obj/tree#2),
    OBJ(tree /obj/tree#3)
}), (: $1->query_name() :));
// ({
//     ({ OBJ(branch /obj/branch#1) }),
//     ({ OBJ(tree /obj/tree#2), OBJ(tree /obj/tree#3) })
// })
```

### SEE ALSO

    filter(3), filter_array(3), map(3), map_array(3)

