---
layout: default
title: master / view_errors
---

### NAME

    view_errors - checks if a user should see error messages

### SYNOPSIS

    int view_errors(object user)

### DESCRIPTION

    if MUDLIB_ERROR_HANDLER is off, view_errors(user) is called when a user
    encounters an error.  If it returns  non-zero,  the  user  is  given  a
    detailed error message.  If not, the default error message is printed.

### SEE ALSO

    MUDLIB_ERROR_HANDER(3)

