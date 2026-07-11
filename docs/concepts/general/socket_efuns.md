---
title: general / socket_efuns
---
# socket_efuns

LPC sockets let an object open network connections directly from LPC, using the
`socket_*` efuns. They make it possible to write network services — telnet
bridges, inter-mud communication, external clients, and so on — entirely in the
mudlib.

Include the mode and option constants with `#include <socket.h>`, and the error
codes with `#include <socket_err.h>`.

Socket I/O is **asynchronous**: efuns like `socket_connect()` and
`socket_write()` return immediately, and the driver later invokes *callback*
functions on your object when network events occur (data arrives, the socket is
writable, the connection closes). You register these callbacks when you create,
connect, or accept a socket.

## Socket modes

`socket_create()` takes a mode that determines the transport and how data is
framed:

| Mode | Value | Transport | Data |
| --- | --- | --- | --- |
| `MUD` | 0 | TCP | Any LPC value except objects (arrays, mappings, etc.), serialized in the driver's save/restore format |
| `STREAM` | 1 | TCP | Raw string data |
| `DATAGRAM` | 2 | UDP | Raw string data |
| `STREAM_BINARY` | 3 | TCP | Binary data as buffers |
| `DATAGRAM_BINARY` | 4 | UDP | Binary data as buffers |
| `STREAM_TLS` | 5 | TLS over TCP | Raw string data, encrypted |
| `STREAM_TLS_BINARY` | 6 | TLS over TCP | Binary data, encrypted |

`MUD` mode is `STREAM` with automatic serialization of LPC values; it is more
convenient but slower and heavier than `STREAM`, so use it only when you need to
exchange structured data. TCP modes (`MUD`, `STREAM`, the TLS modes) are
reliable and ordered. `DATAGRAM` (UDP) is connectionless and unreliable — a
datagram may be silently lost with no error reported — so it is only appropriate
when occasional loss is acceptable.

Note that with `STREAM`, a message may arrive split across several read
callbacks (in order); the receiver must be prepared to reassemble it.

### Serialization in MUD mode

`MUD` mode is the only mode that transfers structured LPC values, and how it does
so matters. It serializes the value with the driver's own save/restore encoding —
the same textual format produced by `save_object()`/`restore_object()` — and puts
a 4-byte length prefix in front of it; the receiving side decodes that back into
an LPC value before invoking the read callback. Arrays, mappings, and nested
combinations are supported (up to a nesting depth of 100). Objects cannot be sent
and yield `EETYPENOTSUPP`.

Because this encoding is specific to the LP driver family, **MUD mode is really
meant for communication between two LP MUDs** that both understand it
(historically, the inter-mud services that shipped with MudOS). A peer that is
not an LP driver receives an opaque, length-prefixed blob and would have to
implement a compatible decoder to use it — there is no generic adapter built in.

The other modes do **not** serialize structured data. `STREAM` and `DATAGRAM`
put strings on the wire as raw bytes; the `*_BINARY` modes send buffers as raw
bytes. Do not expect to hand an array to a `STREAM` socket and have it arrive as
an array: at best you get a raw, non-portable byte dump of its numeric elements,
and mappings are rejected outright. If you need to move structured data between
muds, use `MUD` mode; if you are speaking a byte- or line-oriented protocol
(telnet, HTTP, a custom text protocol), use `STREAM` and handle framing and
encoding yourself.

## Return values and errors

Every socket efun returns a status. `EESUCCESS` (1) means success; any negative
value is an error or warning. `socket_error()` converts an error code to a
human-readable string. The codes are defined in `<socket_err.h>`; common ones
include `EEADDRINUSE` (port already bound), `EEFDRANGE`/`EEBADF` (bad
descriptor), `EESECURITY` (a security check failed), and the flow-control codes
`EECALLBACK`, `EEWOULDBLOCK`, and `EEALREADY` described below.

`socket_create()` and `socket_accept()` are the exceptions: on success they
return a non-negative *socket descriptor* rather than `EESUCCESS`.

## Creating a socket

`socket_create(int mode, string|function read_callback, void|string|function
close_callback)` returns a socket descriptor (>= 0) or a negative error.

Sockets are a finite resource, ultimately bounded by the process's
file-descriptor limit, so always close sockets you are finished with. When an
object is destructed, its sockets are closed automatically. Each open socket has
a unique descriptor; a common idiom is to use it as a key into a mapping of
per-socket state.

## Client/server model

Connection-oriented modes use the client/server model. The **server** creates a
socket, binds it to a well-known port, and listens for connections. The
**client** creates a socket and connects to that port. A port is an integer from
1 to 65535; ports below 1024 are typically reserved, so mudlib services usually
use 1024–65535.

### Server: bind, listen, accept

```c
#include <socket.h>
#include <socket_err.h>

int listen_fd;

void create() {
    listen_fd = socket_create(STREAM, "read_callback", "close_callback");
    if (listen_fd < 0) {
        write("socket_create: " + socket_error(listen_fd) + "\n");
        return;
    }

    int err = socket_bind(listen_fd, 12345);
    if (err != EESUCCESS) {
        write("socket_bind: " + socket_error(err) + "\n");
        socket_close(listen_fd);
        return;
    }

    err = socket_listen(listen_fd, "listen_callback");
    if (err != EESUCCESS) {
        write("socket_listen: " + socket_error(err) + "\n");
        socket_close(listen_fd);
    }
}

// Called when a client connects. Accept it to obtain a new socket
// dedicated to that connection; the listening socket keeps listening.
void listen_callback(int fd) {
    int ns = socket_accept(fd, "read_callback", "write_callback");
    if (ns < 0)
        write("socket_accept: " + socket_error(ns) + "\n");
}

void read_callback(int fd, mixed data) {
    socket_write(fd, "You said: " + data);
}

void write_callback(int fd) {
    // The socket is ready to accept more data (see Flow control).
}

void close_callback(int fd) {
    // The peer closed the connection.
}
```

`socket_bind()`'s port may be `0`, which asks the system to pick any free port —
useful for clients, which do not care which local port they use. Binding a port
already in use fails with `EEADDRINUSE`.

`socket_accept()` returns a **new** descriptor for the accepted connection; the
listening socket is used only to accept, never to transfer data.

### Client: connect

The connection target is a single string of `"address port"`. `socket_connect()`
returns immediately; the outcome arrives later via a callback.

```c
#include <socket.h>
#include <socket_err.h>

int fd;

void create() {
    fd = socket_create(STREAM, "read_callback", "close_callback");
    if (fd < 0) {
        write("socket_create: " + socket_error(fd) + "\n");
        return;
    }

    int err = socket_connect(fd, "138.96.19.14 12345",
                             "read_callback", "write_callback");
    if (err != EESUCCESS) {
        write("socket_connect: " + socket_error(err) + "\n");
        socket_close(fd);
    }
}

void write_callback(int fd) {
    socket_write(fd, "hello");
}

void read_callback(int fd, mixed data) {
    write("Received: " + data + "\n");
}
```

If `socket_connect()` returns `EESUCCESS`, exactly one of three things will
happen later: the read callback fires (data arrived), the write callback fires
(the connection is up and writable), or the close callback fires (the connection
failed or was refused). If it returns an error, no callback will fire.

## Callbacks

Callbacks may be given as function names (strings) or function pointers. Their
signatures are:

```c
void read_callback(int fd, mixed data);              // STREAM / MUD / TLS
void read_callback(int fd, mixed data, string addr); // DATAGRAM modes
void listen_callback(int fd);
void write_callback(int fd);
void close_callback(int fd);
```

For `MUD` mode, `data` may be any LPC type that was sent; it is the receiver's
responsibility to validate it. For `DATAGRAM` modes, the sender's address is
passed as a third argument.

## Flow control

A computer can generate data far faster than a network can send it, so each
socket has a limited send buffer. When it fills, the socket is *flow
controlled* and you must stop writing until it drains. The rule is simple: after
a write reports the buffer is full, wait for the **write callback** before
sending more.

`socket_write()` communicates this through its return value:

- `EESUCCESS` — the data was sent (or buffered with room to spare); you may keep
  writing.
- `EECALLBACK` — the data was buffered but the buffer is now full; stop writing
  until the write callback fires.
- `EEWOULDBLOCK` — the data was **not** buffered; the write callback must fire
  before you retry. Prefer a `call_out()` to retry, giving the system a chance
  to recover.
- `EEALREADY` — you wrote while already flow controlled; the data was not
  buffered. A correctly written application should never see this.

A client is flow controlled until its first write callback, so it must wait for
that callback before its first write. A server may write as soon as it has
accepted the connection.

## Security

Socket use is gated by two mechanisms.

First, the master object's
[`valid_socket()`](../../apply/master/valid_socket) apply is consulted for
every socket operation. It receives the calling object, the operation name, and
an info array `({ fd, owner, address, port })`, and returns 1 to allow or 0 to
deny. If the apply does not exist, access is **denied**. A permissive
implementation looks like:

```c
int valid_socket(object caller, string operation, mixed *info) {
    return 1;
}
```

Second, each socket is *owned* by the object that created it. Socket efuns
compare the caller against the owner and abort if they differ, so one object
cannot operate on another's sockets. A violation of either check returns
`EESECURITY`. Ownership can be transferred deliberately with `socket_release()`
and `socket_acquire()`.

## Efun reference

- [socket_create](../../efun/sockets/socket_create)
- [socket_bind](../../efun/sockets/socket_bind)
- [socket_listen](../../efun/sockets/socket_listen)
- [socket_accept](../../efun/sockets/socket_accept)
- [socket_connect](../../efun/sockets/socket_connect)
- [socket_write](../../efun/sockets/socket_write)
- [socket_close](../../efun/sockets/socket_close)
- [socket_error](../../efun/sockets/socket_error)
- [socket_address](../../efun/sockets/socket_address)
- [socket_status](../../efun/sockets/socket_status)
- [socket_release](../../efun/sockets/socket_release)
- [socket_acquire](../../efun/sockets/socket_acquire)
- [socket_set_option](../../efun/sockets/socket_set_option)
- [socket_get_option](../../efun/sockets/socket_get_option)
