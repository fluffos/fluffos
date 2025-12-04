---
layout: doc
title: sockets / socket_set_option
---
# socket_set_option

### NAME

    socket_set_option - set options on a socket

### SYNOPSIS

    void socket_set_option(int socket, int option, mixed value);

### DESCRIPTION

    Sets socket-specific options that control socket behavior, particularly
    for TLS/SSL connections. This efun allows fine-grained control over
    socket security and connection parameters.

    This function must be called before establishing the connection (i.e.,
    before socket_connect() for client sockets or socket_accept() for
    server sockets).

### ARGUMENTS

- `socket` - The socket descriptor returned by socket_create()
- `option` - The option constant to set (see Options below)
- `value` - The value for the option (type depends on option)

### OPTIONS

#### SO_TLS_VERIFY_PEER (1)

Controls whether the TLS/SSL peer certificate should be verified.

- **Type**: integer
- **Values**:
  - `0` - Do not verify peer certificate (insecure, for testing only)
  - `1` - Verify peer certificate (default, recommended)
- **Use Case**: Client connections to TLS servers

When enabled, the driver will verify that the server's certificate:

- Is signed by a trusted Certificate Authority
- Is not expired
- Matches the hostname being connected to

**Security Note**: Disabling verification (value 0) makes the connection
vulnerable to man-in-the-middle attacks and should only be used for testing.

#### SO_TLS_SNI_HOSTNAME (2)

Sets the Server Name Indication (SNI) hostname for TLS connections.

- **Type**: string
- **Value**: The hostname to send in the SNI extension
- **Use Case**: Client connections when the server hosts multiple TLS sites

SNI allows the server to present the correct SSL certificate when multiple
domains are hosted on the same IP address. This is essential for modern
HTTPS and TLS connections.

#### SO_TLS_CERT (3)

Sets the path to the TLS certificate file for server-side TLS sockets.

- **Type**: string
- **Value**: Path to a PEM-formatted certificate file
- **Use Case**: Server sockets that accept TLS connections

This option is required for TLS server sockets before calling socket_listen().
The certificate file must be in PEM format and readable by the driver.

#### SO_TLS_KEY (4)

Sets the path to the TLS private key file for server-side TLS sockets.

- **Type**: string
- **Value**: Path to a PEM-formatted private key file
- **Use Case**: Server sockets that accept TLS connections

This option is required for TLS server sockets before calling socket_listen().
The private key file must be in PEM format, match the certificate, and be
readable by the driver.

### ERRORS

- Generates an error if the socket descriptor is invalid
- Generates an error if the option is unknown
- Generates an error if the value type doesn't match the option requirements

### EXAMPLES

**Basic TLS client connection with verification:**

```c
void connect_to_server() {
    int sock;

    // Create a TLS client socket
    sock = socket_create(STREAM_TLS, "read_callback", "close_callback");

    // Enable peer verification (default, but shown explicitly)
    socket_set_option(sock, SO_TLS_VERIFY_PEER, 1);

    // Set SNI hostname
    socket_set_option(sock, SO_TLS_SNI_HOSTNAME, "api.example.com");

    // Now connect
    socket_connect(sock, "api.example.com:443", "read_callback", "write_callback");
}
```

**Testing/development with self-signed certificates:**

```c
void connect_to_dev_server() {
    int sock;

    sock = socket_create(STREAM_TLS, "read_callback", "close_callback");

    // ONLY for testing with self-signed certificates!
    // DO NOT USE IN PRODUCTION
    socket_set_option(sock, SO_TLS_VERIFY_PEER, 0);

    socket_set_option(sock, SO_TLS_SNI_HOSTNAME, "dev.example.com");
    socket_connect(sock, "dev.example.com:8443", "read_callback", "write_callback");
}
```

**HTTPS API client:**

```c
void fetch_api_data() {
    int sock;

    sock = socket_create(STREAM_TLS, "api_read", "api_close");

    // Verify the API server's certificate
    socket_set_option(sock, SO_TLS_VERIFY_PEER, 1);

    // Set SNI for the API endpoint
    socket_set_option(sock, SO_TLS_SNI_HOSTNAME, "api.mudserver.com");

    socket_connect(sock, "api.mudserver.com:443", "api_connected");
}

void api_connected(int sock) {
    socket_write(sock, "GET /data HTTP/1.1\r\n"
                      "Host: api.mudserver.com\r\n"
                      "Connection: close\r\n\r\n");
}
```

**TLS server accepting HTTPS connections:**

```c
void create_https_server() {
    int sock;

    // Create a TLS server socket
    sock = socket_create(STREAM_TLS, "read_callback", "close_callback");

    // Set certificate and key for server-side TLS
    socket_set_option(sock, SO_TLS_CERT, "/secure/certs/server.crt");
    socket_set_option(sock, SO_TLS_KEY, "/secure/certs/server.key");

    // Bind to port
    socket_bind(sock, 8443);

    // Start listening for TLS connections
    socket_listen(sock, "listen_callback");
}

void listen_callback(int listen_sock) {
    // Accept the TLS connection (SSL handshake happens automatically)
    int client_sock = socket_accept(listen_sock, "client_read", "client_write");
}

void client_read(int sock, string data) {
    // Data is automatically decrypted
    write("Received encrypted data: " + data);
}
```

### SEE ALSO

- [socket_get_option](socket_get_option.md) - Get socket options
- [socket_create](socket_create.md) - Create a socket
- [socket_connect](socket_connect.md) - Connect a socket
- [socket_accept](socket_accept.md) - Accept connections

### NOTES

**Option Constants:**
The option constants should be defined in your mudlib include files:

```c
#define SO_TLS_VERIFY_PEER    1
#define SO_TLS_SNI_HOSTNAME   2
#define SO_TLS_CERT           3
#define SO_TLS_KEY            4
```

**TLS Socket Modes:**
These options only apply to sockets created with TLS modes:

- `STREAM_TLS` - TLS socket mode
- `STREAM_TLS_BINARY` - TLS binary mode

**Timing:**
Options must be set before the socket is connected. Setting options after
connection establishment may have no effect or cause errors.

**Certificate Verification:**
When SO_TLS_VERIFY_PEER is enabled, the driver uses the system's trusted
Certificate Authority store to verify certificates. Ensure your system's
CA certificates are up to date.

### AVAILABILITY

Added in commit 1fd7f61 (2023). Requires the sockets package to be enabled.
