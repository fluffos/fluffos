---
layout: doc
title: system / sys_network_ports
---
# sys_network_ports

### NAME

    sys_network_ports() - display the active network ports

### SYNOPSIS

    mixed *sys_network_ports(void);

### DESCRIPTION

    Returns an array of arrays; one for each network port that is being actively
    used.

### RETURN VALUES

    An array of the following sub-array will be returned with each active port:

        ({ (int) external_port_#, (string) type, (int) port, (int) tls, })

### SEE ALSO

    network_stats(3)
