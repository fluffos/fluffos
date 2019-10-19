---
layout: default
title: interactive / query_host_name
---

### NAME

    query_host_name() - return the host name

### SYNOPSIS

    string query_host_name( void );

### DESCRIPTION

    query_host_name() returns the name of the host.

### RETURN VALUES

    query_host_name() returns:

        a string hostname on success.

        an empty string on failure.

### SEE ALSO

    resolve(3), socket_address(3), query_ip_name(3), query_ip_number(3)

