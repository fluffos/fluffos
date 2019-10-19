---
layout: default
title: master / valid_bind
---

### NAME

    valid_bind - controls the use of the bind() efun

### SYNOPSIS

    int valid_bind(object binder, object old_owner, object new_owner)

### DESCRIPTION

    This routine is called with 'binder' attempts to use the bind() efun to
    make  a  function  pointer  which  belongs  to  'old_owner'  belong  to
    'new_owner'.  If this routine returns 1, the call is allowed, otherwise
    an error is generated.

