---
layout: default
title: internals / debug_info
---

### NAME

    debug_info() - display debug information

### SYNOPSIS

    mixed debug_info( int operation, ... );
    mixed debug_info( 0, object ob );
    mixed debug_info( 1, object ob );

### DESCRIPTION

    debug_info()  is  a general-purpose facility which may be used to debug
    the MudOS driver.  The debugging information requested is determined by
    the  first  argument.  Successive arguments are determine by the opera‐
    tion selected.

    The existing operations (0 and 1) require a second  object  type  argu‐
    ment, and may be used to display the various fields of the MudOS object
    structure.

    The following LPC code was used to generate the sample output:

    /* di0.c */
    create() {
        debug_info(0, this_object());
    }

        O_HEART_BEAT      : FALSE
        O_IS_WIZARD       : FALSE
        O_ENABLE_COMMANDS : FALSE
        O_CLONE           : FALSE
        O_DESTRUCTED      : FALSE
        O_SWAPPED         : FALSE
        O_ONCE_INTERACTIVE: FALSE
        O_RESET_STATE     : FALSE
        O_WILL_CLEAN_UP   : FALSE
        O_WILL_RESET: TRUE
        total light : 0
        next_reset  : 720300560
        time_of_ref : 720299416
        ref         : 2
        swap_num    : -1
        name        : 'u/c/cynosure/di0'
        next_all    : OBJ(bin/dev/_update)
        This object is the head of the object list.

    /* di1.c */
    create() {
        debug_info(1, this_object());
    }

        program ref's 1
        Name u/c/cynosure/di1.c
        program size 10
        num func's 1 (16)
        num strings 0
        num vars 0 (0)
        num inherits 0 (0)
        total size 104

### SEE ALSO

    dump_file_descriptors(3), dump_socket_status(3)

