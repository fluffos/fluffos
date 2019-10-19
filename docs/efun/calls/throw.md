---
layout: default
title: calls / throw
---

### NAME

    throw() - forces an error to occur in an object.

### SYNOPSIS

    void throw(mixed);

### DESCRIPTION

    The  throw()  efun may be used to force an error to occur in an object.
    When used in conjunction, throw() and catch() allow the  programmer  to
    choose  what  error  message  is displayed when a runtime error occurs.
    When throw() is used,  it  should  be  used  in  conjunction  with  the
    catch(3) efun.  Here is a typical usage:

       string err;
       int rc;

       err = catch(rc = ob->move(dest));
       if (err) {
            throw("move.c: ob->move(dest): " + err + "\n");
            return;
       }

### SEE ALSO

    catch(3), error(3), error_handler(4)

