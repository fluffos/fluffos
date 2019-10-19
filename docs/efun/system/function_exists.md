---
layout: default
title: system / function_exists
---

### NAME

    function_exists()  -  find  the  file containing a given function in an
    object

### SYNOPSYS

    string function_exists( string str, object ob );

### DESCRIPTION

    Return the file name of the object that defines the function  'str'  in
    object  'ob'.  The  returned value can be other than 'file_name(ob)' if
    the function is defined by an inherited object.

    0 is returned if the function was not defined.

    Note that function_exists() does not check shadows.

### SEE ALSO

    call_other(3), call_out(3), functionp(3), valid_shadow(4)

