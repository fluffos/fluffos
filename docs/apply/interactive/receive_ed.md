---
layout: doc
title: interactive / receive_ed
---
# receive_ed

### NAME

    receive_ed - post-process ed editor output

### SYNOPSIS

    mixed receive_ed(string msg, string fname) ;

### DESCRIPTION

    This apply is called to allow post-processing of output from the ed
    line editor before it is sent to the user. This is useful for
    colorizing, formatting, or otherwise modifying ed's output.

    **Arguments:**
    - `msg`: The output message from the ed editor
    - `fname`: The filename being edited, or 0 if not available

    **Return Value:**
    - If this apply returns a non-zero value, the driver will not send
      the original message (the mudlib is expected to handle output)
    - If this apply returns 0 or is not defined, the driver sends the
      message normally

    This apply only works when the `RECEIVE_ED` compile-time option is
    enabled in `local_options`. It is typically used with `OLD_ED` mode.

    Some error messages still bypass this apply and go directly to output.

### EXAMPLE

    ```c
    mixed receive_ed(string msg, string fname) {
        // Colorize ed output
        string colorized = colorize_ed_output(msg);
        receive_message("editor", colorized);
        return 1;  // We handled the output
    }
    ```

### SEE ALSO

    ed(3), receive_message(4)

### NOTE

    This apply requires `RECEIVE_ED` to be defined in `src/local_options`
    at compile time. By default, this option is disabled.
