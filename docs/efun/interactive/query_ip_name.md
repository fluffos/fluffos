---
layout: default
title: interactive / query_ip_name
---

### NAME

    query_ip_name() - return the ip name of a given player object.

### SYNOPSIS

    string query_ip_name( object ob );

### DESCRIPTION

    Return  the  DNS  PTR record for player 'ob's IP .  An asynchronous DNS
    reverse lookup is triggered on new connection to the server.

    Before  lookup  finishes,  this  function  returns   same   result   as
    'query_ip_number(3)'.

    After  lookup  finishes,  if  lookup  is  succesful, this funciton will
    return the DNS PTR value for the IP of this object. If  lookup  failed,
    (due  to  network  issues,  PTR record not configured for this IP etc),
    this function will continue to return  same  result  as  'query_ip_num‐
    ber(3)'.

    The result is cached, there is no overhead for this function.

### SEE ALSO

    query_ip_number(3), query_host_name(3), resolve(3), socket_address(3)

