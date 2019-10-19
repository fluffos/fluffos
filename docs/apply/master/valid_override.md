---
layout: default
title: master / valid_override
---

### NAME

    valid_override - controls the use of efun::

### SYNOPSIS

    int valid_override( string file, string efun_name , string main_file );

### DESCRIPTION

    Add  valid_override  to  master.c  in  order  to control the use of the
    efun:: prefix.  The valid_override function in master.c will be  called
    each  time  the  driver attempts to compile a function call that begins
    with efun::.  If valid_override returns 0, then that compile will fail.
    Thus valid_override provides a way to modify the behavior of efuns that
    isn't circumventable via the efun:: prefix (by having a  simul_efun  of
    the same name as the efun to be modified and having valid_override dis‐
    allow that simul_efun from being overriden).  File will be  the  actual
    file the call appears in; mainfile will be the file being compiled (the
    two can differ due to #include)

    If you wish to have the original 3.1.2 efun:: behavior,  simply  add  a
    line to master.c that looks like this:

       int  valid_override(string  file,  string  efun, string main_file) {
    return 1; }

    Here is an example valid_override that is more restrictive:

      int
      valid_override(string file, string name, string main_file)
      {
          if (file == "/adm/obj/simul_efun") {
              return 1;
          }
          if (name == "destruct")
              return 0;
          if (name == "shutdown")
              return 0;
          if (name == "snoop")
              return 0;
          if (name == "exec")
              return 0;
          return 1;
       }

### AUTHOR

    Truilkan@Basis

### SEE ALSO

    valid_object(4), function_exists(3)

