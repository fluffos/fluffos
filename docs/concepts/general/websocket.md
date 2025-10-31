---
layout: doc
title: WebSocket Support
---
# WebSocket Support

FluffOS provides comprehensive WebSocket support, enabling modern web browsers to connect directly to your MUD without requiring a telnet client. This enables players to access your game through a web page, making it more accessible and convenient.

## Overview

FluffOS WebSocket support includes:
- **Multiple WebSocket protocols** - ASCII, Telnet, Binary, HTTP
- **Secure WebSocket (WSS)** - TLS-encrypted connections
- **Built-in web client** - Modern, full-featured MUD client
- **Protocol compatibility** - Full telnet protocol over WebSocket
- **GMCP support** - Generic Mud Communication Protocol
- **MSP support** - Mud Sound Protocol
- **Static file serving** - Serve HTML/JS/CSS directly from driver
- **Compression** - permessage-deflate support

## WebSocket Server Configuration

### Basic WebSocket Port

Configure WebSocket ports in the driver configuration file:

```
# Non-secure WebSocket
external_port_1: websocket 8080

# Configure directory for static files (web client)
websocket_http_dir: www
```

### Secure WebSocket (WSS)

For production deployments, use WebSocket Secure (WSS) with TLS:

```
# Secure WebSocket
external_port_1: websocket 8443
external_port_1_tls: cert=etc/cert.pem key=etc/key.pem

# Static files directory
websocket_http_dir: www
```

### Multiple WebSocket Ports

You can run both secure and non-secure WebSocket ports:

```
# Regular telnet
port number: 4000

# Non-secure WebSocket (development)
external_port_1: websocket 8080

# Secure WebSocket (production)
external_port_2: websocket 8443
external_port_2_tls: cert=etc/cert.pem key=etc/key.pem

# Static files directory
websocket_http_dir: www
```

### Static Files Directory

The `websocket_http_dir` setting specifies where to serve static web files:

```
# Relative path (from mudlib directory)
websocket_http_dir: www

# Absolute path
websocket_http_dir: /var/www/mud
```

This directory should contain:
- `index.html` - Web client page
- CSS, JavaScript, images, etc.

FluffOS includes a full-featured web client at `src/www/index.html`.

## WebSocket Protocols

FluffOS supports multiple WebSocket subprotocols:

### 1. Telnet Protocol

Full telnet protocol implementation over WebSocket:

```javascript
// Client-side JavaScript
const ws = new WebSocket('ws://mud.example.com:8080', ['telnet']);
```

**Features:**
- Complete telnet option negotiation
- GMCP (Generic Mud Communication Protocol)
- MSP (Mud Sound Protocol)
- MXP support
- NAWS (window size negotiation)
- Terminal type negotiation

**Use case:** Compatible with traditional MUD protocols, supports all telnet features

### 2. ASCII Protocol

Simple text-based protocol without telnet overhead:

```javascript
// Client-side JavaScript
const ws = new WebSocket('ws://mud.example.com:8080', ['ascii']);
```

**Features:**
- Plain text communication
- No telnet protocol overhead
- Lightweight and simple

**Use case:** Simple web clients, debugging, minimal overhead

### 3. Binary Protocol

Legacy protocol for backward compatibility with FluffOS 2.x:

```javascript
// Client-side JavaScript
const ws = new WebSocket('ws://mud.example.com:8080', ['binary']);
```

**Features:**
- Same as telnet protocol
- Maintains compatibility with older clients

**Use case:** Backward compatibility

### 4. HTTP Protocol

Static file serving only:

```javascript
// Access via browser
http://mud.example.com:8080/
```

**Features:**
- Serves files from websocket_http_dir
- Default file: index.html
- No WebSocket connection

**Use case:** Serving the web client interface

## Built-in Web Client

FluffOS includes a modern, full-featured web client at `src/www/index.html`.

### Features

**Connection Management:**
- Configurable server address and port
- Protocol selection (ws:// or wss://)
- Subprotocol selection (ASCII, Telnet, Binary)
- Auto-reconnection with retry logic

**Telnet Protocol Support:**
- Full IAC command handling
- Subnegotiation support
- Option negotiation (WILL/WONT/DO/DONT)
- GMCP message handling
- MSP support
- NAWS (window size reporting)
- Terminal type reporting

**User Interface:**
- ANSI color support
- Command history (↑/↓ arrows)
- Auto-scrolling terminal
- Responsive design (mobile-friendly)
- Dark terminal theme

**Performance:**
- Message buffering (max 1000 messages)
- Efficient ANSI parsing
- Binary data support

### Deploying the Web Client

**Copy to mudlib:**
```bash
# Copy web client to mudlib
cp -r src/www /path/to/mudlib/www

# Or create symlink
ln -s /path/to/fluffos/src/www /path/to/mudlib/www
```

**Configure driver:**
```
# In config file
websocket_http_dir: www
external_port_1: websocket 8080
```

**Access:**
```
http://localhost:8080/
```

### Customizing the Web Client

The web client can be customized by editing `www/index.html`:

**Change default connection:**
```javascript
setDefaultConfig() {
    this.protocolSelect.value = 'ws://';
    this.addressInput.value = 'mud.yourdomain.com';  // Change this
    this.portInput.value = '8080';                   // Change this
    this.subprotocolSelect.value = 'telnet';
}
```

**Modify styling:**
```css
/* Edit the <style> section */
body {
    background-color: #0a0a0a;  /* Change colors */
    color: #e0e0e0;
}
```

**Add custom features:**
- Sound effects with Web Audio API
- Map rendering
- Custom UI panels for GMCP data
- Chat channels
- Macros and hotkeys

## GMCP Over WebSocket

The built-in client supports GMCP (Generic Mud Communication Protocol) for rich client-server communication.

### Server-Side GMCP

Send GMCP messages to clients:

```c
// Check if user supports GMCP
if (query_gmcp(user)) {
    // Send character vitals
    gmcp(user, "Char.Vitals", "{ \"hp\": 100, \"mp\": 50, \"maxhp\": 100, \"maxmp\": 50 }");

    // Send room information
    gmcp(user, "Room.Info", "{ \"name\": \"Market Square\", \"area\": \"City\" }");
}
```

### Client-Side GMCP

The web client automatically handles GMCP negotiation and can process GMCP data:

```javascript
// In www/index.html, modify processGMCPData()
processGMCPData(data) {
    switch (data.module) {
        case 'Char.Vitals':
            // Update health/mana bars
            updateStatusBars(data.data);
            break;
        case 'Room.Info':
            // Update map or room display
            updateRoomDisplay(data.data);
            break;
    }
}
```

## Telnet Protocol Over WebSocket

The telnet protocol implementation supports all standard telnet options:

### Supported Telnet Options

- **ECHO (1)** - Server echo control
- **SUPPRESS_GO_AHEAD (3)** - Suppress go-ahead
- **TERMINAL_TYPE (24)** - Terminal identification
- **NAWS (31)** - Window size negotiation
- **NEW_ENVIRON (39)** - Environment variables
- **CHARSET (42)** - Character set negotiation

### Supported MUD Protocols

- **MSDP (69)** - Mud Server Data Protocol
- **MSSP (70)** - Mud Server Status Protocol
- **MCCP (86)** - Mud Client Compression Protocol
- **MSP (90)** - Mud Sound Protocol
- **MXP (91)** - Mud eXtension Protocol
- **ZMP (93)** - Zenith Mud Protocol
- **GMCP (201)** - Generic Mud Communication Protocol

### Example: Telnet Negotiation

```javascript
// Client sends WILL NAWS
IAC WILL NAWS

// Server responds DO NAWS
IAC DO NAWS

// Client sends window size
IAC SB NAWS <width-high> <width-low> <height-high> <height-low> IAC SE
```

The built-in client handles all negotiation automatically.

## Security Considerations

### Use WSS in Production

Always use secure WebSocket (WSS) for production:

```
# Production configuration
external_port_1: websocket 8443
external_port_1_tls: cert=/etc/letsencrypt/live/mud.example.com/fullchain.pem \
                     key=/etc/letsencrypt/live/mud.example.com/privkey.pem
```

### CORS and Security Headers

WebSocket connections respect browser security policies. For cross-origin connections, ensure your server is configured correctly.

### Input Validation

Always validate input from WebSocket connections:

```c
void receive_message(string msg) {
    // Validate and sanitize input
    if (!msg || strlen(msg) > MAX_INPUT_LENGTH) {
        return;
    }

    // Process command
    process_command(msg);
}
```

### Rate Limiting

Implement rate limiting to prevent abuse:

```c
// In receive_message apply
void receive_message(string msg) {
    if (query_command_rate() > MAX_COMMANDS_PER_SECOND) {
        write("You are sending commands too quickly.\n");
        return;
    }

    process_command(msg);
}
```

## Performance Optimization

### Compression

FluffOS automatically negotiates compression (permessage-deflate) with clients that support it. This reduces bandwidth usage significantly.

### Message Buffering

Buffer messages to reduce WebSocket overhead:

```c
// Buffer multiple messages
string buffer = "";
buffer += "Line 1\n";
buffer += "Line 2\n";
buffer += "Line 3\n";

// Send once
write(buffer);
```

### Connection Limits

Configure connection limits in the driver config:

```
# Maximum connections
max players : 100

# Connection timeout
time to reset : 900
time to cleanup : 3600
```

## Troubleshooting

### Common Issues

**1. WebSocket connection fails:**
```
Error: WebSocket connection failed
```
**Solution:** Check firewall rules allow WebSocket port
```bash
# Allow port through firewall
sudo ufw allow 8080/tcp
sudo ufw allow 8443/tcp
```

**2. Static files not served:**
```
Error: 404 Not Found
```
**Solution:** Verify websocket_http_dir path
```
# Check path is correct
websocket_http_dir: www

# Verify files exist
ls -la /path/to/mudlib/www/index.html
```

**3. WSS certificate errors:**
```
Error: Certificate verification failed
```
**Solution:** Use valid certificate (Let's Encrypt)
```bash
# Generate certificate
sudo certbot certonly --standalone -d mud.example.com
```

**4. GMCP not working:**
```c
// Verify GMCP is enabled
if (!query_gmcp(user)) {
    // Client doesn't support GMCP
    write("Your client does not support GMCP.\n");
}
```

**5. Connection drops randomly:**
- Check for network issues
- Increase timeout values
- Implement heartbeat/ping mechanism

### Testing WebSocket Connections

**Test with browser console:**
```javascript
// Open browser console (F12)
const ws = new WebSocket('ws://localhost:8080', ['telnet']);
ws.onopen = () => console.log('Connected!');
ws.onmessage = (e) => console.log('Received:', e.data);
ws.send('look\n');
```

**Test with command-line tool:**
```bash
# Install wscat
npm install -g wscat

# Connect to WebSocket
wscat -c ws://localhost:8080 -s telnet

# Send commands
> look
> who
```

**Check server logs:**
```c
// Enable WebSocket debugging
debug_level(DEBUG_WEBSOCKET);
```

## Advanced Usage

### Custom WebSocket Protocols

You can implement custom WebSocket subprotocols by modifying the driver source.

### Integration with Web Frameworks

Serve the MUD client alongside a web application:

**Nginx reverse proxy:**
```nginx
server {
    listen 80;
    server_name mud.example.com;

    # Web application
    location / {
        proxy_pass http://webapp:3000;
    }

    # WebSocket for MUD
    location /ws {
        proxy_pass http://fluffos:8080;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "Upgrade";
    }
}
```

### Mobile Support

The built-in client is mobile-friendly and responsive. For better mobile experience:

**Add to home screen:**
```html
<!-- In index.html <head> -->
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="mobile-web-app-capable" content="yes">
<link rel="apple-touch-icon" href="icon.png">
```

**Optimize for touch:**
```css
/* Larger touch targets */
.command-input {
    font-size: 16px;  /* Prevents zoom on iOS */
    padding: 12px;
}
```

## See Also

- [TLS Support](tls.md) - Secure WebSocket configuration
- [Driver Configuration](../../driver/config.md) - WebSocket port setup
- [socket_create(3)](../../efun/sockets/socket_create.md) - Socket functions
- [gmcp(3)](../../efun/interactive/gmcp.md) - GMCP efun
- [query_gmcp(3)](../../efun/interactive/query_gmcp.md) - Check GMCP support

## References

- [RFC 6455](https://tools.ietf.org/html/rfc6455) - WebSocket Protocol
- [libwebsockets](https://libwebsockets.org/) - WebSocket library used by FluffOS
- [GMCP Specification](https://www.gammon.com.au/gmcp) - Generic Mud Communication Protocol
- [Telnet Protocol](https://tools.ietf.org/html/rfc854) - Telnet specification
- [Telnet Options](https://www.iana.org/assignments/telnet-options/telnet-options.xhtml) - IANA registry
