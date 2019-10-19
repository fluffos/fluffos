---
layout: default
title: objects / set_hide
---

### NAME

    set_hide() - set the hide flag on a hidable object

### SYNOPSIS

    void set_hide( int flag );

### DESCRIPTION

    Sets  the  hidden  flag  of an object to 'flag', which should be 0 or 1
    (hide disable, or hide enable, respectively).  Only objects  for  which
    'master()->valid_hide(ob)'  returns  true  may  make themselves hidden.
    When the object is hidden, only other hidable objects will be  able  to
    find  the object with find_object(), or multiple-object returning efuns
    such as users(), children(), all_inventory(), etc.

### SEE ALSO

    valid_hide(4)

