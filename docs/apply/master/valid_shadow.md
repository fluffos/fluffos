---
layout: default
title: master / valid_shadow
---

### NAME

    valid_shadow - controls which objects can be shadowed

### SYNOPSIS

    int valid_shadow( object ob );

### DESCRIPTION

    When  an  object  attempts  to  shadow  'ob'  (with the shadow() efun),
    valid_shadow in the master object is called.  One object  parameter  is
    passed,  which  is  the  object that previous_object() is attempting to
    shadow.  valid_shadow() should return 0 if the  shadow  should  not  be
    permitted,  in which case the shadow() call will return 0 and fail.  If
    valid_shadow() returns 1, the shadow is allowed.

### SEE ALSO

    shadow(3), query_shadowing(3)

