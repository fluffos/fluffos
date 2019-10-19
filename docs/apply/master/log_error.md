---
layout: default
title: master / log_error
---

### NAME

    log_error - log errors intelligently

### SYNOPSIS

    void log_error( string file, string message );

### DESCRIPTION

    Whenever  an error occurs during compilation, the function log_error in
    the master object is called with the filename that the  error  occurred
    in  and  the error message itself.  Then, log_error is free to do what‐
    ever it thinks it should do with that  information.   Usually  this  is
    deciding  based  on  the  filename  where  the  error message should be
    logged, and then writing it to that file.

### SEE ALSO

    error_handler(4)

### AUTHOR

    Wayfarer@Portals

