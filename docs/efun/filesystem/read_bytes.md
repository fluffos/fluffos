---
layout: default
title: filesystem / read_bytes
---

### NAME

    read_bytes()  -  reads  a contiguous series of bytes from a file into a
    string

### SYNOPSIS

    string read_bytes( string path, int start, int length );

### DESCRIPTION

    This function reads 'length' bytes beginning at byte # 'start'  in  the
    file  named  'path'.   The  bytes  are returned as a string.  Note that
    (start + length) must  not  be  past  the  end  of  the  file  or  else
    read_bytes  will  fail.  If the second and third arguments are omitted,
    the entire file is returned.

### SEE ALSO

    read_file(3), write_bytes(3)

