---
layout: default
title: contrib / query_ip_port.pre
---

### NAME

    query_ip_port - returns local_port of connection

### SYNOPSIS

    int query_ip_port(void | object);

### DESCRIPTION

    object defaults to this_player()

    returns local_port of connection or 0 if object not interactive
