---
layout: default
title: master / get_include_path
---

### NAME

    get_include_path - decide include path based on object

### SYNOPSIS

    mixed get_include_path( string object_path );

### DESCRIPTION

    When  the  driver  starts to compile a new object this apply is used to
    determine what directories are to be searched for  include  files.   In
    contrast  to the static run time configuration option 'include directo‐
    ries' this allows for a dynamic approach based on the storage place.

    The argument to this apply is the absolute path of  the  object  to  be
    compiled, return value is an array of strings with it's elements either
    the absolute path to the desired include  directories  or  the  special
    string ":DEFAULT:" which will be replaced be the include path as set by
    the runtime configuration of the driver.

### EXAMPLE

    string *get_include_path(string file) {
        string *t = explode(file, "/");
        if(t[1] == "Domain")    // Domains have their own include directory
            return ({ "/Domain/" + t[2] + "/include", ":DEFAULT:" });
        else                    // use default for others
            return ({ ":DEFAULT:" }); }

