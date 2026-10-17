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
    `mixed *args` argument, those values are passed to the body instead.

    See [Hooks](/concepts/general/hooks).

### SEE ALSO
    hook_attach(3), hook_detach(3)
