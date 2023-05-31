---
layout: default
title: contrib / network_ports
---

### NAME

    network_ports() - display the active network ports

### SYNOPSIS

    mixed *network_ports(void);

### DESCRIPTION

    Returns an array of arrays; one for each network port that is being actively
    used.

### RETURN VALUES

    An array of the following sub-array will be returned for each active port:

        ({ (string) type, (int) port, (int) tls, })

### SEE ALSO

    network_stats(3)
