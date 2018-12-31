---
layout: default
title: contrib / query_ip_port.pre
---

int query_ip_port(void | object);

object defaults to this_player()

returns local_port of connection or 0 if object not interactive
