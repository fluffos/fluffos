---
layout: default
title: interactive / enable_commands
---

### NAME

    enable_commands() - allow object to use 'player' commands

### SYNOPSIS

    Deprecated:
      void enable_commands( void );

    Since FluffOS 3.0-alpha7.1:
      void enable_commands( int setup_actions = 0 );

### DESCRIPTION

    enable_commands() marks this_object() as a living object, and allows it
    to use commands added with add_action()  (by  using  command()).   When
    enable_commands()  is called, the driver also looks for the local func‐
    tion catch_tell(), and if found, it will call it every time  a  message
    (via say() for example) is given to the object.

    Since  FluffOS  3.0-alpha7: This function now accept int, default to 0.
    which has same meaning of old form.  which merely re-enables  commands,
    but  don't  setup actions. (it should have been cleared when previously
    called disable_commands())

    When setup_actions > 0, Driver will re-setup all the actions by calling
    init()  on  its  environment,  sibling  and inventory objects. (in that
    order).

### BUGS

    Do not call this function in any other place than create()  or  strange
    things will likely occur.

### SEE ALSO

    this_object(3),  living(3),  add_action(3),  command(3), catch_tell(4),
    say(3), create(4)

