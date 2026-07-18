#!/usr/bin/env node
// dap-ws-bridge.js -- generic stdio<->WebSocket bridge for the FluffOS
// debugger (src/debugger/DESIGN.md).
//
// The driver speaks plain DAP JSON messages over a "dap" websocket
// subprotocol (one message per ws text frame, no Content-Length framing).
// Most DAP clients other than a purpose-built extension (nvim-dap, and any
// editor that only knows how to spawn a stdio debug adapter) expect the
// standard DAP stdio transport instead: each message prefixed with
// "Content-Length: N\r\n\r\n". This script is that adapter -- it does no
// protocol interpretation, just re-framing in both directions, so it stays
// correct even as the debugger protocol gains new request/event types.
//
// Usage:
//   node tools/dap-ws-bridge.js --host 127.0.0.1 --port 4711
//
// Point your DAP client's "debug adapter executable" at this script (with
// the same args) instead of a TCP debugServer. Authentication (the
// "attach" request's token argument) is entirely up to the client's own
// launch/attach configuration -- the bridge does not know or care about it.
//
// No npm dependencies.

'use strict';

const net = require('net');
const crypto = require('crypto');

function parseArgs(argv) {
  const out = { host: '127.0.0.1', port: null };
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    if (a === '--host') out.host = argv[++i];
    else if (a === '--port') out.port = parseInt(argv[++i], 10);
    else if (a === '--help' || a === '-h') { printUsage(); process.exit(0); }
  }
  return out;
}

function printUsage() {
  console.error('Usage: node tools/dap-ws-bridge.js --host <host> --port <port>');
}

// ---- tiny websocket client (RFC6455 client side, no deps) ---------------
// Kept in sync in spirit with tools/ws-smoke.js and tools/dap-smoke.js;
// duplicated rather than shared because each script is meant to run
// standalone with zero install step.

class WSClient {
  constructor(socket, host, port, subprotocol) {
    this.sock = socket;
    this.buf = Buffer.alloc(0);
    this._onFrame = null;
    this._pending = [];
    this.onClose = () => {};
    this.established = false;
    const key = crypto.randomBytes(16).toString('base64');
    this.acceptWant = crypto.createHash('sha1')
      .update(key + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11').digest('base64');
    socket.write(
      `GET / HTTP/1.1\r\nHost: ${host}:${port}\r\nUpgrade: websocket\r\n` +
      `Connection: Upgrade\r\nSec-WebSocket-Key: ${key}\r\n` +
      `Sec-WebSocket-Version: 13\r\nSec-WebSocket-Protocol: dap\r\n\r\n`);
    socket.on('data', (d) => this.feed(d));
    socket.on('close', () => this.onClose());
    socket.on('error', () => this.onClose());
  }

  feed(d) {
    this.buf = Buffer.concat([this.buf, d]);
    if (!this.established) {
      const end = this.buf.indexOf('\r\n\r\n');
      if (end === -1) return;
      const head = this.buf.slice(0, end).toString('latin1');
      this.buf = this.buf.slice(end + 4);
      if (!/HTTP\/1\.1 101/.test(head)) {
        process.stderr.write('dap-ws-bridge: upgrade refused:\n' + head + '\n');
        this.sock.destroy();
        return;
      }
      const accept = /sec-websocket-accept:\s*(\S+)/i.exec(head);
      if (!accept || accept[1] !== this.acceptWant) {
        process.stderr.write('dap-ws-bridge: bad Sec-WebSocket-Accept\n');
        this.sock.destroy();
        return;
      }
      this.established = true;
    }
    for (;;) {
      if (this.buf.length < 2) return;
      const b0 = this.buf[0], b1 = this.buf[1];
      const opcode = b0 & 0x0f;
      let len = b1 & 0x7f, off = 2;
      if (len === 126) {
        if (this.buf.length < 4) return;
        len = this.buf.readUInt16BE(2); off = 4;
      } else if (len === 127) {
        if (this.buf.length < 10) return;
        len = Number(this.buf.readBigUInt64BE(2)); off = 10;
      }
      if (b1 & 0x80) off += 4;
      if (this.buf.length < off + len) return;
      const payload = this.buf.slice(off, off + len);
      this.buf = this.buf.slice(off + len);
      if (opcode === 0x9) { this.sendFrame(0xA, payload); continue; }
      if (opcode === 0x8) { this.sock.destroy(); this.onClose(); return; }
      if (opcode === 0x1 || opcode === 0x2 || opcode === 0x0) {
        if (this._onFrame) this._onFrame(payload);
        else this._pending.push(payload);
      }
    }
  }

  get onFrame() { return this._onFrame; }
  set onFrame(fn) {
    this._onFrame = fn;
    if (fn && this._pending.length) {
      const q = this._pending;
      this._pending = [];
      for (const payload of q) fn(payload);
    }
  }

  sendFrame(opcode, payload) {
    const mask = crypto.randomBytes(4);
    let head;
    if (payload.length < 126) {
      head = Buffer.from([0x80 | opcode, 0x80 | payload.length]);
    } else if (payload.length < 65536) {
      head = Buffer.alloc(4);
      head[0] = 0x80 | opcode; head[1] = 0x80 | 126;
      head.writeUInt16BE(payload.length, 2);
    } else {
      head = Buffer.alloc(10);
      head[0] = 0x80 | opcode; head[1] = 0x80 | 127;
      head.writeBigUInt64BE(BigInt(payload.length), 2);
    }
    const masked = Buffer.from(payload);
    for (let i = 0; i < masked.length; i++) masked[i] ^= mask[i & 3];
    this.sock.write(Buffer.concat([head, mask, masked]));
  }

  sendText(str) { this.sendFrame(0x1, Buffer.from(str, 'utf8')); }
}

// ---- stdio <-> DAP Content-Length framing --------------------------------

class StdioFramer {
  constructor(onMessage) {
    this.buf = Buffer.alloc(0);
    this.onMessage = onMessage;
  }
  feed(chunk) {
    this.buf = Buffer.concat([this.buf, chunk]);
    for (;;) {
      const headerEnd = this.buf.indexOf('\r\n\r\n');
      if (headerEnd === -1) return;
      const header = this.buf.slice(0, headerEnd).toString('utf8');
      const m = /Content-Length:\s*(\d+)/i.exec(header);
      if (!m) {
        process.stderr.write('dap-ws-bridge: malformed stdio frame header\n');
        this.buf = Buffer.alloc(0);
        return;
      }
      const len = parseInt(m[1], 10);
      const bodyStart = headerEnd + 4;
      if (this.buf.length < bodyStart + len) return;
      const body = this.buf.slice(bodyStart, bodyStart + len).toString('utf8');
      this.buf = this.buf.slice(bodyStart + len);
      this.onMessage(body);
    }
  }
}

function writeFramed(text) {
  const body = Buffer.from(text, 'utf8');
  process.stdout.write(`Content-Length: ${body.length}\r\n\r\n`);
  process.stdout.write(body);
}

// ---- main -----------------------------------------------------------------

function main() {
  const args = parseArgs(process.argv.slice(2));
  if (!args.port) {
    printUsage();
    process.exit(2);
  }

  const sock = net.connect(args.port, args.host);
  sock.on('error', (e) => {
    process.stderr.write(`dap-ws-bridge: connection error: ${e.message}\n`);
    process.exit(1);
  });

  sock.once('connect', () => {
    const ws = new WSClient(sock, args.host, args.port, 'dap');
    ws.onFrame = (payload) => writeFramed(payload.toString('utf8'));
    ws.onClose = () => process.exit(0);

    const framer = new StdioFramer((text) => ws.sendText(text));
    process.stdin.on('data', (chunk) => framer.feed(chunk));
    process.stdin.on('end', () => sock.destroy());
  });
}

main();
