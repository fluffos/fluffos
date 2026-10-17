---
title: hooks / hook_list
---
# hook_list

### NAME
    hook_list - enumerate live hook attachments

### SYNOPSIS
    mapping *hook_list(object | void owner);

### DESCRIPTION
    Returns an array of the same mappings that
    [hook_query](/efun/hooks/hook_query) produces. With no argument, every
    live attachment is listed. With an `owner`, only that object's
    attachments are returned.

    See [Hooks](/concepts/general/hooks).

### SEE ALSO
    hook_attach(3), hook_query(3), hook_detach(3)
