---
layout: default
title: interactive / resolve
---

### NAME

    resolve() - resolve an internet address to domain name, or vice versa

### SYNOPSIS

    int resolve( string address, string callback_func );

### DESCRIPTION

    resolve()  resolves  'address',  which should be an internet address in
    the form "127.0.0.1" or a domain name, into its domain name, or  inter‐
    net  address.   When  the  resolve is complete, 'callback_func' will be
    called in the current object.  The form of the callback is:

    void callback(string address, string resolved, int key);

    'key' will match  up  with  the  number  that  the  call  to  resolve()
    returned.   'address'  will  be  the  domain  name  of  the  host,  and
    'resolved' the dotted decimal ip address.  The unknown value will be  0
    if the lookup failed.

### SEE ALSO

    query_host_name(3),  socket_address(3), query_ip_name(3), query_ip_num‐
    ber(3)

