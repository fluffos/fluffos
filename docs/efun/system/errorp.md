---
layout: default
title: system / errorp
---

### NAME

    errorp() - determine whether or not a given variable is an error code

### SYNOPSIS

    int errorp( mixed arg );

### DESCRIPTION

    Returns 1 if 'arg' is an integer that is an error code.

    Eventually  efuns  will  be  modified to return standard error codes so
    that code like this will be possible:

        if (errorp(result = efun_call()))
            printf("error = %d\n", result);

    In future, there will also be a perror(result)  efun  to  return  error
    string that goes with a particular error integer.

### SEE ALSO

    nullp(3), undefinedp(3)

