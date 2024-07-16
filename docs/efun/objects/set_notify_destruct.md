---
layout: doc
title: objects / set_notify_destruct
---
# set_notify_destruct

### NAME

    set_notify_destruct() - set the object to be notified when it is destructed

### SYNOPSIS

    void set_notify_destruct(int flag);

### DESCRIPTION

    Toggles a flag in an object that determines whether or not it will be
    notified when it is destructed. If the flag is set to 1, the object will
    receive a call to the function on_destruct() when it is destructed. If the
    flag is set to 0, the object will not receive this notification.

    Objects do not receive this notification by default. To receive the call to
    on_destruct(), the object must call set_notify_destruct(1) at some point
    during its lifetime.

    The set_notify_destruct() efun may only be called from within the object
    that would like to receive the notification.

### SEE ALSO

    query_notify_destruct, on_destruct, destruct
