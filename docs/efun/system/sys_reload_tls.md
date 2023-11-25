---
layout: doc
title: system / sys_reload_tls
---
# sys_reload_tls

### NAME

    sys_reload_tls() - Reload the TLS certificate and key for given port.

### SYNOPSIS

    void sys_reload_tls ( int port_index );

### DESCRIPTION

    Reload the TLS certificate and key for given port index, as specified in config file.

    For example, if you defined external_port_1_tls in the config then you can calling sys_reload_tls(1) to reload
    it. This allows you to update the certificate and key without restarting the server.

    Note:
      - you have to overwrite the previous cert/key file on disk before calling this function.
      - reloading TLS for websocket port is not currently supported.
