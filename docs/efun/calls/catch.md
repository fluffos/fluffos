---
layout: default
title: calls / catch
---

### NAME

    catch() - catch an evaluation error

### SYNOPSIS

    mixed catch( mixed expr );

### DESCRIPTION

    Evaluate  <expr>.  If  there  is no error, 0 is returned. If there is a
    standard error, a string (with a leading '*') will be returned.

    The function throw() can also be used to immediately return any  value,
    except 0. catch() is not really a function call, but a directive to the
    compiler.

    The catch() is somewhat costly, and should not be used  just  anywhere.
    Rather, use it at places where an error would destroy consistency.

### SEE ALSO

    error(3), throw(3), error_handler(4)

