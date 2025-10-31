---
layout: doc
title: TLS/SSL Support
---
# TLS/SSL Support

FluffOS provides comprehensive TLS (Transport Layer Security) support for secure network communications. TLS encrypts data between clients and the server, protecting against eavesdropping and tampering.

## Overview

FluffOS TLS support includes:
- **Secure telnet connections** - Encrypted telnet over TLS
- **Secure WebSocket connections** - WSS (WebSocket Secure) protocol
- **Client-side TLS sockets** - Connect to external HTTPS/TLS servers
- **Certificate-based authentication** - Verify server identities
- **Modern TLS versions** - Support for TLS 1.2 and 1.3
- **SNI (Server Name Indication)** - Multi-domain support

## TLS for Server Ports

### Configuration

TLS is configured in the driver configuration file for external ports.

#### Basic TLS Port Configuration

```
# Secure telnet on port 4443
external_port_1: telnet 4443
external_port_1_tls: cert=etc/cert.pem key=etc/key.pem

# Secure WebSocket on port 8443
external_port_2: websocket 8443
external_port_2_tls: cert=etc/cert.pem key=etc/key.pem
```

#### Multiple TLS Ports

```
# Primary telnet (non-TLS)
port number : 4000

# Secure telnet
external_port_1: telnet 4443
external_port_1_tls: cert=etc/certs/mud.pem key=etc/certs/mud.key

# Secure WebSocket for web client
external_port_2: websocket 8443
external_port_2_tls: cert=etc/certs/web.pem key=etc/certs/web.key

# Regular WebSocket (development)
external_port_3: websocket 8080
```

### Certificate Files

TLS requires two files:
1. **Certificate file (.pem/.crt)** - Public certificate
2. **Private key file (.key/.pem)** - Private key

Both can be combined in a single .pem file or kept separate.

## Generating Certificates

### Self-Signed Certificates (Development/Testing)

**Quick generation:**
```bash
openssl req -x509 -newkey rsa:4096 \
  -keyout key.pem -out cert.pem \
  -days 365 -nodes \
  -subj "/CN=localhost"
```

**With more details:**
```bash
openssl req -x509 -newkey rsa:4096 \
  -keyout key.pem -out cert.pem \
  -days 365 -nodes \
  -subj "/C=US/ST=State/L=City/O=MyMUD/CN=mud.example.com"
```

**Combined certificate + key:**
```bash
openssl req -x509 -newkey rsa:4096 \
  -keyout combined.pem -out combined.pem \
  -days 365 -nodes \
  -subj "/CN=mud.example.com"

# Use in config
external_port_1_tls: cert=etc/combined.pem key=etc/combined.pem
```

**Notes on self-signed certificates:**
- ⚠️ Browsers will show security warnings
- ⚠️ Not suitable for production
- ✅ Fine for development and testing
- ✅ Good for private/internal MUDs

### Production Certificates (Let's Encrypt)

For production MUDs with a domain name, use Let's Encrypt for free, trusted certificates:

**Installation (Ubuntu/Debian):**
```bash
sudo apt-get install certbot
```

**Generate certificate:**
```bash
sudo certbot certonly --standalone \
  -d mud.yourdomain.com \
  --email admin@yourdomain.com
```

**Certificate location:**
```
/etc/letsencrypt/live/mud.yourdomain.com/fullchain.pem  # Certificate
/etc/letsencrypt/live/mud.yourdomain.com/privkey.pem    # Private key
```

**FluffOS configuration:**
```
external_port_1: telnet 4443
external_port_1_tls: cert=/etc/letsencrypt/live/mud.yourdomain.com/fullchain.pem \
                     key=/etc/letsencrypt/live/mud.yourdomain.com/privkey.pem
```

**Auto-renewal:**
```bash
# Test renewal
sudo certbot renew --dry-run

# Set up auto-renewal (cron)
sudo crontab -e
# Add: 0 3 * * * certbot renew --post-hook "systemctl reload fluffos"
```

**Certificate renewal without downtime:**
```bash
# After certbot renews certificate
# Reload TLS certificates without restarting
# In-game as admin:
sys_reload_tls(1);  // Reload TLS for port 1
```

## Client Connections to TLS Servers

### Socket TLS Modes

FluffOS supports TLS for outbound socket connections:

```c
// Available socket modes
STREAM        // Regular TCP
STREAM_BINARY // Binary TCP
STREAM_TLS    // TLS encrypted
STREAM_TLS_BINARY  // TLS encrypted binary
```

### Basic TLS Client Connection

```c
void connect_to_api() {
    int sock;

    // Create TLS socket
    sock = socket_create(STREAM_TLS, "read_callback", "close_callback");

    // Configure TLS options
    socket_set_option(sock, SO_TLS_VERIFY_PEER, 1);  // Verify certificate
    socket_set_option(sock, SO_TLS_SNI_HOSTNAME, "api.example.com");

    // Connect
    socket_connect(sock, "api.example.com:443", "connected_callback");
}

void connected_callback(int sock) {
    // Connection established, send HTTPS request
    socket_write(sock,
        "GET /api/data HTTP/1.1\r\n"
        "Host: api.example.com\r\n"
        "User-Agent: FluffOS-MUD\r\n"
        "Connection: close\r\n\r\n"
    );
}

void read_callback(int sock, mixed data) {
    // Process HTTPS response
    write("Received: " + data + "\n");
}

void close_callback(int sock) {
    write("Connection closed.\n");
}
```

### HTTPS Client Example

```c
// Complete HTTPS client
void fetch_url(string url, function callback) {
    string host, path;
    int port = 443;

    // Parse URL
    if (sscanf(url, "https://%s/%s", host, path) != 2) {
        return callback(0, "Invalid URL");
    }

    // Create TLS socket
    int sock = socket_create(STREAM_TLS, "http_read", "http_close");

    // Configure TLS with certificate verification
    socket_set_option(sock, SO_TLS_VERIFY_PEER, 1);
    socket_set_option(sock, SO_TLS_SNI_HOSTNAME, host);

    // Store callback for later
    set_socket_callback(sock, callback);

    // Connect
    socket_connect(sock, host + ":" + port,
        lambda(({'sock}), ({
            #'socket_write, 'sock,
            "GET /" + path + " HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Connection: close\r\n\r\n"
        }))
    );
}
```

### TLS Socket Options

#### SO_TLS_VERIFY_PEER

Controls certificate verification:

```c
// Enable verification (recommended)
socket_set_option(sock, SO_TLS_VERIFY_PEER, 1);

// Disable verification (insecure, testing only)
socket_set_option(sock, SO_TLS_VERIFY_PEER, 0);
```

**When to disable verification:**
- ⚠️ Testing with self-signed certificates
- ⚠️ Development environments
- ⚠️ **NEVER in production!**

**Security implications:**
- Disabled = vulnerable to man-in-the-middle attacks
- Disabled = cannot verify server identity
- Enabled = secure, authenticated connection

#### SO_TLS_SNI_HOSTNAME

Server Name Indication for multi-domain servers:

```c
// Required for servers hosting multiple domains
socket_set_option(sock, SO_TLS_SNI_HOSTNAME, "api.example.com");
```

**Why SNI is important:**
- Servers can host multiple TLS sites on one IP
- SNI tells server which certificate to use
- Required for most modern HTTPS APIs
- Equivalent to HTTP Host header

## TLS Protocols and Ciphers

### Supported TLS Versions

FluffOS uses OpenSSL and supports:
- **TLS 1.2** - Widely supported, secure
- **TLS 1.3** - Modern, faster, more secure
- **SSL 3.0, TLS 1.0, TLS 1.1** - Disabled by default (insecure)

The driver automatically negotiates the best available version.

### Cipher Suites

FluffOS uses OpenSSL's default secure cipher suites, which include:
- **ECDHE-RSA-AES256-GCM-SHA384** - Strong, modern
- **ECDHE-RSA-AES128-GCM-SHA256** - Fast, secure
- **DHE-RSA-AES256-GCM-SHA384** - Strong forward secrecy

Weak ciphers (RC4, DES, 3DES, MD5) are automatically disabled.

## Security Best Practices

### Certificate Management

**DO:**
- ✅ Use trusted certificates for production (Let's Encrypt)
- ✅ Keep private keys secure (chmod 600)
- ✅ Renew certificates before expiration
- ✅ Use strong keys (RSA 4096 or ECDSA 256)
- ✅ Enable certificate verification in clients

**DON'T:**
- ❌ Commit private keys to git
- ❌ Use self-signed certs in production
- ❌ Disable certificate verification in production
- ❌ Share private keys between servers
- ❌ Use weak keys (RSA < 2048)

### Configuration Security

```bash
# Secure certificate file permissions
chmod 600 etc/key.pem
chmod 644 etc/cert.pem
chown muduser:muduser etc/*.pem
```

### Client Security

```c
// GOOD: Verify server certificate
socket_set_option(sock, SO_TLS_VERIFY_PEER, 1);
socket_set_option(sock, SO_TLS_SNI_HOSTNAME, "trusted-api.com");

// BAD: Skip verification (vulnerable!)
socket_set_option(sock, SO_TLS_VERIFY_PEER, 0);  // DON'T DO THIS IN PRODUCTION
```

## Troubleshooting

### Common Issues

**1. Certificate errors on client connection:**
```
Error: certificate verify failed
```
**Solution:** Check certificate chain, ensure CA certificates are installed
```bash
# Update CA certificates (Ubuntu/Debian)
sudo apt-get install ca-certificates
sudo update-ca-certificates
```

**2. Self-signed certificate rejection:**
```c
// For testing only
socket_set_option(sock, SO_TLS_VERIFY_PEER, 0);
```

**3. SNI not set:**
```
Error: wrong version number / unexpected EOF
```
**Solution:** Set SNI hostname:
```c
socket_set_option(sock, SO_TLS_SNI_HOSTNAME, "api.example.com");
```

**4. Mixed content (non-TLS to TLS):**
- Ensure all external connections use appropriate protocol
- Use TLS sockets for HTTPS endpoints
- Use regular sockets for HTTP endpoints

**5. Certificate file not found:**
```
Error: No such file or directory
```
**Solution:** Use absolute paths or paths relative to mudlib directory
```
# Absolute path
external_port_1_tls: cert=/etc/ssl/certs/mud.pem key=/etc/ssl/private/mud.key

# Relative to mudlib directory
external_port_1_tls: cert=etc/certs/mud.pem key=etc/certs/mud.key
```

### Testing TLS Connections

**Test server TLS:**
```bash
# Test telnet TLS
openssl s_client -connect mud.example.com:4443

# Test WebSocket TLS
openssl s_client -connect mud.example.com:8443
```

**Test certificate validity:**
```bash
# Check certificate expiration
openssl x509 -in cert.pem -text -noout | grep "Not After"

# Verify certificate chain
openssl verify -CAfile ca.pem cert.pem
```

**Test in-game:**
```c
// Test outbound TLS connection
void test_tls() {
    int sock = socket_create(STREAM_TLS, "test_read", "test_close");
    socket_set_option(sock, SO_TLS_VERIFY_PEER, 1);
    socket_set_option(sock, SO_TLS_SNI_HOSTNAME, "www.google.com");
    socket_connect(sock, "www.google.com:443", "test_connect");
}
```

## Performance Considerations

### TLS Overhead

- **CPU:** 5-15% additional CPU usage for encryption/decryption
- **Memory:** Minimal (~50KB per connection)
- **Latency:** <1ms added latency for handshake
- **Throughput:** Minimal impact on modern CPUs

### Optimization Tips

1. **Use TLS 1.3** - Faster handshakes, better performance
2. **Session resumption** - Handled automatically by OpenSSL
3. **Hardware acceleration** - Use AES-NI capable CPUs
4. **Connection reuse** - Keep connections alive when possible

## See Also

- [socket_create(3)](../../efun/sockets/socket_create.md) - Create sockets
- [socket_set_option(3)](../../efun/sockets/socket_set_option.md) - Configure TLS
- [socket_get_option(3)](../../efun/sockets/socket_get_option.md) - Query TLS settings
- [sys_reload_tls(3)](../../efun/system/sys_reload_tls.md) - Reload certificates
- [Driver Configuration](../../driver/config.md) - TLS port setup
- [WebSocket Support](websocket.md) - WebSocket with TLS

## References

- [OpenSSL Documentation](https://www.openssl.org/docs/)
- [Let's Encrypt](https://letsencrypt.org/) - Free TLS certificates
- [TLS 1.3 RFC](https://tools.ietf.org/html/rfc8446)
- [Mozilla TLS Configuration](https://wiki.mozilla.org/Security/Server_Side_TLS)
