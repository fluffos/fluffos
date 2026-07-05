---
layout: doc
title: TLS/SSL Support
---
# TLS/SSL Support

FluffOS provides TLS (Transport Layer Security) support for secure network
communications. TLS encrypts data between clients and the server, protecting
against eavesdropping and tampering.

## Overview

FluffOS TLS support includes:

- **Secure telnet connections** — encrypted telnet over TLS
- **Secure WebSocket connections** — WSS (WebSocket Secure)
- **Client-side TLS sockets** — connect to external HTTPS/TLS servers
- **Certificate verification** — verify server identities against the system CA store
- **Modern TLS versions** — negotiates the highest mutually supported version (typically TLS 1.2 or 1.3)
- **SNI (Server Name Indication)** — select a certificate by hostname

## TLS for Server Ports

### Configuration

Listening ports are configured with numbered `external_port_N` entries in the
driver configuration file. TLS is enabled on a listener by adding a matching
`external_port_N_tls` line giving a certificate and key. See the
[driver configuration](../../driver/config.md) for the full port syntax.

```
# Secure telnet on port 4443
external_port_1 : telnet 4443
external_port_1_tls : cert=etc/cert.pem key=etc/key.pem

# Secure WebSocket on port 8443
external_port_2 : websocket 8443
external_port_2_tls : cert=etc/cert.pem key=etc/key.pem
```

The `cert=` and `key=` pair must be on a single line; the parser reads each
config line whole and does not support backslash continuation.

Certificate and key paths are passed to OpenSSL as-is, so relative paths are
resolved from the **driver's working directory** (not the mudlib). Use absolute
paths if there is any ambiguity about where the driver is started.

#### Multiple TLS Ports

```
# Legacy single telnet port; equivalent to external_port_1 : telnet 4000
port number : 4000

# Secure telnet
external_port_1 : telnet 4443
external_port_1_tls : cert=etc/certs/mud.pem key=etc/certs/mud.key

# Secure WebSocket for a web client
external_port_2 : websocket 8443
external_port_2_tls : cert=etc/certs/web.pem key=etc/certs/web.key

# Regular WebSocket (development)
external_port_3 : websocket 8080
```

### Certificate Files

TLS requires two things:

1. **Certificate** (`.pem`/`.crt`) — the public certificate (chain).
2. **Private key** (`.key`/`.pem`) — the private key.

They can be two separate files or a single combined PEM. For a combined file,
point both `cert=` and `key=` at the same path.

## Generating Certificates

### Self-Signed Certificates (Development/Testing)

Quick generation:

```bash
openssl req -x509 -newkey rsa:4096 \
  -keyout key.pem -out cert.pem \
  -days 365 -nodes \
  -subj "/CN=localhost"
```

With more details:

```bash
openssl req -x509 -newkey rsa:4096 \
  -keyout key.pem -out cert.pem \
  -days 365 -nodes \
  -subj "/C=US/ST=State/L=City/O=MyMUD/CN=mud.example.com"
```

To produce a single combined PEM, generate the two files and concatenate them:

```bash
cat cert.pem key.pem > combined.pem

# Use in config
external_port_1_tls : cert=etc/combined.pem key=etc/combined.pem
```

Notes on self-signed certificates:

- Browsers will show security warnings.
- Not suitable for production.
- Fine for development, testing, and private/internal muds.

### Production Certificates (Let's Encrypt)

For production muds with a domain name, use Let's Encrypt for free, trusted
certificates.

Installation (Ubuntu/Debian):

```bash
sudo apt-get install certbot
```

Generate a certificate:

```bash
sudo certbot certonly --standalone \
  -d mud.yourdomain.com \
  --email admin@yourdomain.com
```

Certificate location:

```
/etc/letsencrypt/live/mud.yourdomain.com/fullchain.pem  # Certificate
/etc/letsencrypt/live/mud.yourdomain.com/privkey.pem    # Private key
```

FluffOS configuration (`cert=`/`key=` on one line):

```
external_port_1 : telnet 4443
external_port_1_tls : cert=/etc/letsencrypt/live/mud.yourdomain.com/fullchain.pem key=/etc/letsencrypt/live/mud.yourdomain.com/privkey.pem
```

After a renewal the certificate files change on disk, but the running driver
keeps using the certificate it loaded at boot. Reload it in-game without
restarting using the [`sys_reload_tls()`](../../efun/system/sys_reload_tls.md)
efun, which takes the 1-based external port number:

```c
sys_reload_tls(1);  // reload TLS for external_port_1
```

(Reloading WebSocket TLS this way is not supported; only stream/telnet ports.)

You can drive this from a certbot deploy hook that triggers an in-game admin
command, or from a scheduled `call_out` that reloads after renewals.

## Client Connections to TLS Servers

### Socket TLS Modes

Two socket modes carry TLS:

```c
STREAM_TLS         // TLS-encrypted stream (raw string data)
STREAM_TLS_BINARY  // TLS-encrypted stream (binary buffer data)
```

### TLS Client Connection

`socket_connect()` takes a **numeric IP address** and does **not** perform DNS
resolution, so resolve the hostname first with the asynchronous `resolve()`
efun, then connect to the address it returns. The connection address is a single
`"IP port"` string (space-separated), and `socket_connect()` takes four
arguments: the socket, the address, a read callback, and a write callback. TLS
options must be set before connecting.

```c
#include <socket.h>
#include <socket_err.h>

void connect_to_api() {
    // resolve() is asynchronous; it calls "on_resolved" with the result.
    resolve("api.example.com", "on_resolved");
}

// resolve() callback: (queried name, numeric IP or 0 on failure, query id)
void on_resolved(string name, string ip, int key) {
    if (!ip) {
        write("Could not resolve " + name + "\n");
        return;
    }

    int sock = socket_create(STREAM_TLS, "read_callback", "close_callback");
    if (sock < 0) {
        write("socket_create: " + socket_error(sock) + "\n");
        return;
    }

    // Configure TLS before connecting.
    socket_set_option(sock, SO_TLS_VERIFY_PEER, 1);   // verify the server cert
    socket_set_option(sock, SO_TLS_SNI_HOSTNAME, name);

    // Address is "IP port", space-separated. socket_connect takes read AND
    // write callbacks.
    int err = socket_connect(sock, ip + " 443",
                             "read_callback", "write_callback");
    if (err != EESUCCESS) {
        write("socket_connect: " + socket_error(err) + "\n");
        socket_close(sock);
    }
}

// Fires once the TLS handshake completes and the socket is writable.
void write_callback(int sock) {
    socket_write(sock,
        "GET /api/data HTTP/1.1\r\n"
        "Host: api.example.com\r\n"     // Host header still uses the hostname
        "User-Agent: FluffOS\r\n"
        "Connection: close\r\n\r\n");
}

void read_callback(int sock, mixed data) {
    // STREAM data may arrive in several pieces; a real HTTP client should
    // accumulate it and parse once the full response has arrived.
    write("Received: " + data + "\n");
}

void close_callback(int sock) {
    write("Connection closed.\n");
}
```

Only the socket connection itself uses the numeric IP; the SNI option and the
HTTP `Host` header still use the hostname.

### TLS Socket Options

TLS options are set with `socket_set_option()` before `socket_connect()`.

#### SO_TLS_VERIFY_PEER

Controls certificate verification against the system CA store:

```c
socket_set_option(sock, SO_TLS_VERIFY_PEER, 1);  // verify (recommended)
socket_set_option(sock, SO_TLS_VERIFY_PEER, 0);  // do not verify (insecure)
```

Disabling verification leaves the connection vulnerable to man-in-the-middle
attacks and unable to confirm the server's identity. Only disable it for local
testing against self-signed certificates, never in production.

#### SO_TLS_SNI_HOSTNAME

Server Name Indication tells a server that hosts multiple TLS sites on one IP
which certificate to present. It is required by most modern HTTPS endpoints and
is roughly the TLS-layer equivalent of the HTTP `Host` header.

```c
socket_set_option(sock, SO_TLS_SNI_HOSTNAME, "api.example.com");
```

## TLS Protocols and Ciphers

FluffOS uses OpenSSL and negotiates the highest TLS version supported by both
ends — typically **TLS 1.2** or **TLS 1.3**. The driver explicitly disables
SSLv2, SSLv3, and TLS 1.1 for both server and client contexts; the remaining
version floor is governed by the linked OpenSSL build's security level.

Cipher selection follows OpenSSL's defaults, which favor modern AEAD suites
(for example ECDHE with AES-GCM) and exclude long-obsolete ciphers such as RC4
and single DES.

## Security Best Practices

### Certificate Management

Do:

- Use trusted certificates for production (e.g. Let's Encrypt).
- Keep private keys readable only by the mud user (`chmod 600`).
- Renew certificates before they expire, and reload with `sys_reload_tls()`.
- Use strong keys (RSA 4096 or ECDSA P-256).
- Enable certificate verification (`SO_TLS_VERIFY_PEER`) in clients.

Don't:

- Commit private keys to version control.
- Use self-signed certificates in production.
- Disable certificate verification in production.
- Share one private key across unrelated servers.
- Use weak keys (RSA < 2048).

### File Permissions

```bash
chmod 600 etc/key.pem
chmod 644 etc/cert.pem
chown muduser:muduser etc/*.pem
```

## Troubleshooting

**Certificate verify failed (client):** the server's certificate could not be
validated against the local CA store. Ensure the system CA certificates are
installed and up to date:

```bash
sudo apt-get install ca-certificates
sudo update-ca-certificates
```

For a self-signed server during testing only, you may disable verification:

```c
socket_set_option(sock, SO_TLS_VERIFY_PEER, 0);
```

**"wrong version number" / unexpected EOF:** frequently caused by a missing SNI
hostname, or by connecting with TLS to a plaintext port (or vice versa). Set the
SNI hostname and confirm the port actually speaks TLS:

```c
socket_set_option(sock, SO_TLS_SNI_HOSTNAME, "api.example.com");
```

**Certificate file not found:** the driver could not open the `cert=`/`key=`
path. Because paths are relative to the driver's working directory, prefer
absolute paths:

```
external_port_1_tls : cert=/etc/ssl/certs/mud.pem key=/etc/ssl/private/mud.key
```

### Testing TLS Connections

Test a server port from the shell:

```bash
openssl s_client -connect mud.example.com:4443
```

Check a certificate's validity:

```bash
# Expiration date
openssl x509 -in cert.pem -text -noout | grep "Not After"

# Verify against a CA
openssl verify -CAfile ca.pem cert.pem
```

Test an outbound TLS connection in-game using the client pattern shown above
(resolve the host, then `socket_connect()` to the numeric IP with `SO_TLS_*`
options set).

## Performance Considerations

TLS adds CPU cost for the handshake and for encrypting and decrypting data, plus
a small amount of memory per connection. On modern hardware — especially CPUs
with AES-NI — this overhead is generally negligible at mud scale. Prefer TLS 1.3
for faster handshakes; OpenSSL handles session resumption automatically. Keeping
connections alive avoids repeating handshakes.

## See Also

- [socket_create](../../efun/sockets/socket_create.md) — create sockets
- [socket_set_option](../../efun/sockets/socket_set_option.md) — configure TLS
- [socket_get_option](../../efun/sockets/socket_get_option.md) — query TLS settings
- [sys_reload_tls](../../efun/system/sys_reload_tls.md) — reload certificates
- [socket_efuns](socket_efuns.md) — LPC sockets overview
- [Driver Configuration](../../driver/config.md) — TLS port setup
- [WebSocket Support](websocket.md) — WebSocket with TLS

## References

- [OpenSSL Documentation](https://www.openssl.org/docs/)
- [Let's Encrypt](https://letsencrypt.org/) — free TLS certificates
- [TLS 1.3 RFC](https://tools.ietf.org/html/rfc8446)
- [Mozilla Server-Side TLS](https://wiki.mozilla.org/Security/Server_Side_TLS)
