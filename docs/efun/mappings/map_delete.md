---
layout: default
title: mappings / map_delete
---

### NAME

    map_delete()  -  remove a (key, value) pair from a mapping based on the
    key

### SYNOPSIS

    void map_delete( mapping m, mixed element );

### DESCRIPTION

    map_delete removes the (key, value) from the mapping  m  that  has  key
    equal to element.

    For example, given:

        mapping names;

        names = ([]);
        names["truilkan"] = "john";
        names["wayfarer"] = "erik";
        names["jacques"] = "dwayne";

    Then:

        map_delete(names,"truilkan");

    causes the mapping 'names' to be equal to:

        (["wayfarer" : "erik", "jacques" : "dwayne"])

    keys(names)  will  not  contain "truilkan" after map_delete(names,"tru‐
    ilkan") is called [unless ("truilkan", *) is subsequently added back to
    the mapping].

### SEE ALSO

    allocate_mapping(3)

