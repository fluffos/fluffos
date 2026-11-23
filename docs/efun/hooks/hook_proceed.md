---
title: hooks / hook_proceed
---
# hook_proceed

### NAME
    hook_proceed - run the wrapped call from a HOOK_AROUND handler

### SYNOPSIS
    mixed hook_proceed(void | mixed *args);

### DESCRIPTION
    Valid only inside a `HOOK_AROUND` handler attached to the `function`
    join point. Runs the real LPC function once and returns its result.
    Calling it a second time, or outside an around handler, is an error.

    With no argument, the original call arguments are used. With a
    `mixed *args` argument, those values are passed to the body instead;
    missing trailing arguments take their declared defaults, and a varargs
    function receives its extra arguments as an array, as in any call.

    If the body raises an error, hook_proceed() raises the same error in
    the handler, which may catch it. If the handler does not, the error
    reaches the original caller, and the body is not run a second time.

    Errors if the target object was destructed or recompiled since the
    call was made.

    See [Hooks](/concepts/general/hooks).

### SEE ALSO
    hook_attach(3), hook_detach(3)
