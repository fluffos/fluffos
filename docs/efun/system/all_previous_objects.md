---
layout: default
title: system / all_previous_objects
---

### NAME

    all_previous_objects()  -  returns  an array of objects that called the
    current function

### SYNOPSIS

    object *all_previous_objects();

### DESCRIPTION

    Returns an array of objects that called current  function.   Note  that
    local  function  calls  do  not  set  previous_object()  to the current
    object, but leave it unchanged.

### SEE ALSO

    call_other(3), call_out(3), origin(3), previous_object(3)

