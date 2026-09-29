---
title: ed / ed_start
---
# ed_start

### NAME

    ed_start() - start an ed session

### SYNOPSIS

    string ed_start(string file | void, int restricted | void,
                    int scroll_lines | void)

### DESCRIPTION

    This efun is available only if __OLD_ED__ is not defined.

    The  internal editor is started, optionally loading 'file' for editing.
    The resulting output is returned.  The editor session  remains  active,
    and further calls to ed_cmd() can be used to send commands to it.

    If restricted is 1, the commands that change which file is being edited
    will be disabled.

    If scroll_lines is nonzero, it sets the number of lines displayed by the
    editor's scrolling commands (such as 'z').  The default is 20.

    When ed_start() is called with exactly two arguments, a second argument
    equal to 1 is treated as 'restricted'; any other value is treated as
    'scroll_lines'.

    Only one ed session can be active per object at a time.

