---
layout: doc
title: objects / query_notify_destruct
---
# query_notify_destruct

### NAME

    query_notify_destruct() - determine if an object will be notified when it
    is destructed

### SYNOPSIS

    int query_notify_destruct(void | object ob);

### DESCRIPTION

    The query_notify_destruct() efun is used to determine if an object will be
    notified when it is destructed. If the object is set to be notified, the
    function will return 1. If the object is not set to be notified, the
    function will return 0.

    If no argument is provided, the efun will query the current object. If an
    object is provided as an argument, the efun will query that object.

    Objects do not receive this notification by default. To receive the call to
    on_destruct(), the object must call set_notify_destruct(1) at some point
    during its lifetime.

    The query_notify_destruct() efun may only be called on any object.

### SEE ALSO

    set_notify_destruct, on_destruct, destruct
