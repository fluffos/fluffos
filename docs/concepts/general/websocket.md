---
title: WebSocket Support
---
# WebSocket Support

FluffOS supports WebSocket connections, letting modern web browsers connect
directly to your mud without a separate telnet client. Players can reach the
game from a web page, and the driver can serve the client itself.

## Overview

FluffOS WebSocket support includes:

- **Multiple subprotocols** — `telnet`, `ascii`, and `binary`
- **Secure WebSocket (WSS)** — TLS-encrypted connections
- **Built-in web client** — shipped in `src/www`
- **Full telnet protocol** — option negotiation, GMCP, MSP, MXP, and more over the `telnet` subprotocol
- **Static file serving** — serve the client's HTML/JS/CSS from the driver
- **Compression** — `permessage-deflate` when the client supports it

## WebSocket Server Configuration

WebSocket listeners are configured like any other port, with the `websocket`
protocol keyword. A WebSocket port additionally **requires** the
`websocket http dir` option, which names the directory of static files to serve.
See the [driver configuration](../../driver/config.md) for the full port
syntax.

### Basic WebSocket Port

```
# Non-secure WebSocket
external_port_1 : websocket 8080

# Directory of static files (the web client)
websocket http dir : www
```

### Secure WebSocket (WSS)

For production, enable TLS on the port with a matching `external_port_N_tls`
line. The `cert=`/`key=` pair must be on a single line (the config parser reads
each line whole and does not support backslash continuation).

```
external_port_1 : websocket 8443
external_port_1_tls : cert=etc/cert.pem key=etc/key.pem
websocket http dir : www
```

### Multiple WebSocket Ports

```
# Regular telnet
port number : 4000

# Non-secure WebSocket (development)
external_port_1 : websocket 8080

# Secure WebSocket (production)
external_port_2 : websocket 8443
external_port_2_tls : cert=etc/cert.pem key=etc/key.pem

websocket http dir : www
```

### Static Files Directory

`websocket http dir` is passed to the web server as-is, so a relative path is
resolved from the **driver's working directory** (use an absolute path if that
is ambiguous). It should contain an `index.html` plus any CSS, JavaScript, and
images. FluffOS ships a full-featured client in `src/www`.

```
# Relative to the driver's working directory
websocket http dir : www

# Absolute path
websocket http dir : /var/www/mud
```

## WebSocket Subprotocols

A browser selects a subprotocol as the second argument to `new WebSocket()`.
FluffOS offers three:

### telnet

The full telnet protocol over WebSocket — option negotiation plus the mud
protocols listed below (GMCP, MSP, MXP, etc.). This is the subprotocol the
built-in client uses.

```javascript
const ws = new WebSocket('ws://mud.example.com:8080', ['telnet']);
```

### ascii

Plain text with no telnet framing — lightweight, useful for simple clients and
debugging.

```javascript
const ws = new WebSocket('ws://mud.example.com:8080', ['ascii']);
```

### binary

Handled identically to the `telnet` subprotocol; retained for compatibility with
older clients.

```javascript
const ws = new WebSocket('ws://mud.example.com:8080', ['binary']);
```

A WebSocket port also serves the static files from `websocket http dir` over
plain HTTP, so browsing to `http://host:port/` returns `index.html` (this is
ordinary file serving, not a WebSocket connection).

## Built-in Web Client

FluffOS ships a modern web client in `src/www` (`index.html`). It provides:

- Connection management — configurable address/port, `ws://` or `wss://`, and subprotocol selection
- Full telnet handling — IAC commands, subnegotiation, option negotiation, GMCP, MSP, NAWS, terminal type
- A terminal UI — ANSI color, command history, auto-scroll, and a responsive, mobile-friendly layout

### Deploying the Web Client

Make the client available under your `websocket http dir`:

```bash
# Copy the client into place
cp -r src/www /path/to/mudlib/www

# Or symlink it
ln -s /path/to/fluffos/src/www /path/to/mudlib/www
```

```
websocket http dir : www
external_port_1 : websocket 8080
```

Then browse to `http://localhost:8080/`.

### Customizing the Web Client

The client is a single `index.html`. Two useful entry points:

- `setDefaultConfig()` sets the default address, port, and subprotocol.
- `processGMCPData(data)` handles incoming GMCP messages — extend it to drive
  status bars, maps, or custom panels.

```javascript
setDefaultConfig() {
    this.protocolSelect.value = 'ws://';
    this.addressInput.value = 'mud.yourdomain.com';
    this.portInput.value = '8080';
    this.subprotocolSelect.value = 'telnet';
}
```

## GMCP Over WebSocket

Over the `telnet` subprotocol the client negotiates GMCP (Generic Mud
Communication Protocol), so the standard GMCP efuns work unchanged.

### Server-Side GMCP

`send_gmcp()` sends to the **current interactive object**, so call it from within
the user's object (it takes a single string containing the package name followed
by JSON). `has_gmcp()` reports whether a given interactive object negotiated
GMCP (defaulting to the current object).

```c
// Called in the context of the user object
if (has_gmcp()) {
    send_gmcp("Char.Vitals { \"hp\": 100, \"mp\": 50, \"maxhp\": 100, \"maxmp\": 50 }");
    send_gmcp("Room.Info { \"name\": \"Market Square\", \"area\": \"City\" }");
}
```

### Client-Side GMCP

The web client parses GMCP messages and dispatches them through
`processGMCPData()`:

```javascript
processGMCPData(data) {
    switch (data.module) {
        case 'Char.Vitals':
            updateStatusBars(data.data);
            break;
        case 'Room.Info':
            updateRoomDisplay(data.data);
            break;
    }
}
```

## Telnet Protocol Over WebSocket

The `telnet` subprotocol supports the standard telnet options and the common mud
protocols. Negotiation is handled by the driver's telnet layer, identically to a
raw telnet connection.

### Supported telnet options

- **ECHO (1)** — echo control
- **SUPPRESS_GO_AHEAD (3)**
- **TERMINAL_TYPE (24)**
- **NAWS (31)** — window size
- **NEW_ENVIRON (39)** — environment variables
- **CHARSET (42)** — character set negotiation

### Supported mud protocols

- **MSDP (69)** — Mud Server Data Protocol
- **MSSP (70)** — Mud Server Status Protocol
- **MCCP2 (86)** — Mud Client Compression Protocol
- **MSP (90)** — Mud Sound Protocol
- **MXP (91)** — Mud eXtension Protocol
- **ZMP (93)** — Zenith Mud Protocol
- **GMCP (201)** — Generic Mud Communication Protocol

The built-in client performs all negotiation automatically.

## Security Considerations

### Use WSS in Production

Always serve production traffic over WSS (see [TLS Support](tls.md)):

```
external_port_1 : websocket 8443
external_port_1_tls : cert=/etc/letsencrypt/live/mud.example.com/fullchain.pem key=/etc/letsencrypt/live/mud.example.com/privkey.pem
```

### Validating Player Input

Input from a WebSocket client arrives through the **same path as any interactive
connection** — the normal command and input handling on the user object — not
through `receive_message` (which is the apply for *output* sent to the user).
Validate and rate-limit player input where you process commands, exactly as you
would for a telnet connection; there is nothing WebSocket-specific to do.

## Performance

### Compression

FluffOS advertises `permessage-deflate` and uses it with any client that
supports it, reducing bandwidth for large or repetitive output.

## Troubleshooting

**Connection fails:** confirm the port is open through any firewall.

```bash
sudo ufw allow 8080/tcp
sudo ufw allow 8443/tcp
```

**Static files 404:** verify `websocket http dir` points at the directory that
contains `index.html`.

```
websocket http dir : www
```

**WSS certificate errors:** use a certificate the client trusts (e.g. Let's
Encrypt); see [TLS Support](tls.md).

**GMCP not working:** confirm the client negotiated it.

```c
if (!has_gmcp(user)) {
    write("Your client does not support GMCP.\n");
}
```

**Enable WebSocket debug logging:** the driver has a `websocket` debug category.

```c
set_debug_level("websocket");
```

### Testing WebSocket Connections

From a browser console:

```javascript
const ws = new WebSocket('ws://localhost:8080', ['telnet']);
ws.onopen = () => console.log('Connected!');
ws.onmessage = (e) => console.log('Received:', e.data);
ws.send('look\n');
```

From the command line with `wscat`:

```bash
npm install -g wscat
wscat -c ws://localhost:8080 -s telnet
```

## Reverse Proxying

You can place a WebSocket port behind a web server, for example to serve the mud
client alongside a website or to terminate TLS at the proxy:

```nginx
server {
    listen 80;
    server_name mud.example.com;

    location / {
        proxy_pass http://webapp:3000;
    }

    location /ws {
        proxy_pass http://fluffos:8080;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "Upgrade";
    }
}
```

## See Also

- [TLS Support](tls.md) — secure WebSocket configuration
- [Driver Configuration](../../driver/config.md) — WebSocket port setup
- [send_gmcp](../../efun/interactive/send_gmcp.md) — send a GMCP message
- [has_gmcp](../../efun/interactive/has_gmcp.md) — check GMCP support

## References

- [RFC 6455](https://tools.ietf.org/html/rfc6455) — the WebSocket protocol
- [libwebsockets](https://libwebsockets.org/) — the WebSocket library FluffOS uses
- [GMCP Specification](https://www.gammon.com.au/gmcp)
- [Telnet Protocol (RFC 854)](https://tools.ietf.org/html/rfc854)
- [Telnet Options (IANA)](https://www.iana.org/assignments/telnet-options/telnet-options.xhtml)
