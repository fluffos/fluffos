// FluffOS LPC Debugger -- VS Code extension.
//
// Thin by design: the driver speaks DAP directly over a WebSocket (subprotocol
// "dap"; see src/debugger/DESIGN.md), so this extension's only real job is
// (1) opening that WebSocket and relaying DAP JSON messages 1:1 through VS
// Code's DebugAdapterInlineImplementation, and (2) mapping mudlib-absolute
// source paths ("/std/room.lpc") to and from local workspace paths, so
// breakpoints set in the editor and stack frames shown by the driver line up
// with real files on disk.
//
// No protocol logic lives here beyond that -- VS Code's built-in DAP client
// renders breakpoints, the call stack, variables, etc. from the driver's
// responses exactly as it would for any other debugger.

'use strict';

const vscode = require('vscode');
const net = require('net');
const crypto = require('crypto');
const path = require('path');

// ---- tiny websocket client (RFC6455 client side, no deps) ---------------
// Same minimal implementation as tools/dap-smoke.js / tools/dap-ws-bridge.js
// (duplicated intentionally: this extension ships standalone, no npm install
// step, and each of the three scripts needs to keep working on its own).

class WSClient {
  constructor(socket, host, port) {
    this.sock = socket;
    this.buf = Buffer.alloc(0);
    this._onFrame = null;
    this.onClose = () => {};
    this.onError = () => {};
    this.established = false;
    const key = crypto.randomBytes(16).toString('base64');
    this.acceptWant = crypto
      .createHash('sha1')
      .update(key + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11')
      .digest('base64');
    socket.write(
      `GET / HTTP/1.1\r\nHost: ${host}:${port}\r\nUpgrade: websocket\r\n` +
        `Connection: Upgrade\r\nSec-WebSocket-Key: ${key}\r\n` +
        `Sec-WebSocket-Version: 13\r\nSec-WebSocket-Protocol: dap\r\n\r\n`
    );
    socket.on('data', (d) => this.feed(d));
    socket.on('close', () => this.onClose());
    socket.on('error', (e) => this.onError(e));
  }

  feed(d) {
    this.buf = Buffer.concat([this.buf, d]);
    if (!this.established) {
      const end = this.buf.indexOf('\r\n\r\n');
      if (end === -1) return;
      const head = this.buf.slice(0, end).toString('latin1');
      this.buf = this.buf.slice(end + 4);
      if (!/HTTP\/1\.1 101/.test(head)) {
        this.onError(new Error('websocket upgrade refused:\n' + head));
        this.sock.destroy();
        return;
      }
      const accept = /sec-websocket-accept:\s*(\S+)/i.exec(head);
      if (!accept || accept[1] !== this.acceptWant) {
        this.onError(new Error('bad Sec-WebSocket-Accept'));
        this.sock.destroy();
        return;
      }
      this.established = true;
    }
    for (;;) {
      if (this.buf.length < 2) return;
      const b0 = this.buf[0],
        b1 = this.buf[1];
      const opcode = b0 & 0x0f;
      let len = b1 & 0x7f,
        off = 2;
      if (len === 126) {
        if (this.buf.length < 4) return;
        len = this.buf.readUInt16BE(2);
        off = 4;
      } else if (len === 127) {
        if (this.buf.length < 10) return;
        len = Number(this.buf.readBigUInt64BE(2));
        off = 10;
      }
      if (b1 & 0x80) off += 4;
      if (this.buf.length < off + len) return;
      const payload = this.buf.slice(off, off + len);
      this.buf = this.buf.slice(off + len);
      if (opcode === 0x9) {
        this.sendFrame(0xa, payload);
        continue;
      }
      if (opcode === 0x8) {
        this.sock.destroy();
        this.onClose();
        return;
      }
      if (opcode === 0x1 || opcode === 0x2 || opcode === 0x0) {
        if (this._onFrame) this._onFrame(payload);
      }
    }
  }

  set onFrame(fn) {
    this._onFrame = fn;
  }

  sendFrame(opcode, payload) {
    const mask = crypto.randomBytes(4);
    let head;
    if (payload.length < 126) {
      head = Buffer.from([0x80 | opcode, 0x80 | payload.length]);
    } else if (payload.length < 65536) {
      head = Buffer.alloc(4);
      head[0] = 0x80 | opcode;
      head[1] = 0x80 | 126;
      head.writeUInt16BE(payload.length, 2);
    } else {
      head = Buffer.alloc(10);
      head[0] = 0x80 | opcode;
      head[1] = 0x80 | 127;
      head.writeBigUInt64BE(BigInt(payload.length), 2);
    }
    const masked = Buffer.from(payload);
    for (let i = 0; i < masked.length; i++) masked[i] ^= mask[i & 3];
    this.sock.write(Buffer.concat([head, mask, masked]));
  }

  sendText(str) {
    this.sendFrame(0x1, Buffer.from(str, 'utf8'));
  }

  close() {
    try {
      this.sock.destroy();
    } catch (_) {
      /* already gone */
    }
  }
}

// ---- mudlib-path <-> local-path mapping ----------------------------------

class PathMapper {
  constructor(remoteRoot, localRoot) {
    this.remoteRoot = (remoteRoot || '/').replace(/\/+$/, '') || '/';
    this.localRoot = localRoot || '';
  }

  // "/std/room.lpc" -> "<localRoot>/std/room.lpc"
  toLocal(remotePath) {
    if (!remotePath) return remotePath;
    let rel = remotePath;
    if (this.remoteRoot !== '/' && rel.startsWith(this.remoteRoot)) {
      rel = rel.slice(this.remoteRoot.length);
    }
    rel = rel.replace(/^\/+/, '');
    return path.join(this.localRoot, rel);
  }

  // local workspace path -> mudlib-absolute path
  toRemote(localPath) {
    if (!localPath) return localPath;
    let rel = path.relative(this.localRoot, localPath);
    if (rel.startsWith('..')) {
      // Outside the mapped root: pass through unchanged (best effort; the
      // driver will report a `source` fetch failure if it's genuinely wrong).
      return localPath;
    }
    rel = rel.split(path.sep).join('/');
    const root = this.remoteRoot === '/' ? '' : this.remoteRoot;
    return root + '/' + rel;
  }

  // Rewrite every `source.path` this DAP message contains, in the given
  // direction. Message shapes are heterogeneous (requests carry `arguments`,
  // responses carry `body`, events carry `body`), so this walks generically
  // rather than special-casing each command.
  rewrite(message, direction) {
    const map = direction === 'toRemote' ? this.toRemote.bind(this) : this.toLocal.bind(this);
    const visit = (node) => {
      if (Array.isArray(node)) {
        node.forEach(visit);
        return;
      }
      if (node && typeof node === 'object') {
        if (typeof node.path === 'string' && ('name' in node || 'sourceReference' in node || Object.keys(node).length <= 3)) {
          // Heuristic: a DAP `Source` object always has `path` alongside
          // `name`/`sourceReference`/nothing else -- narrow enough to avoid
          // touching unrelated string fields that happen to be named `path`.
          node.path = map(node.path);
        }
        for (const k of Object.keys(node)) visit(node[k]);
      }
    };
    visit(message);
    return message;
  }
}

// ---- the inline debug adapter ---------------------------------------------

class FluffosDebugAdapter {
  constructor(config) {
    this._onDidSendMessage = new vscode.EventEmitter();
    this.onDidSendMessage = this._onDidSendMessage.event;
    this.config = config;
    const localRoot =
      config.localRoot && config.localRoot !== '${workspaceFolder}'
        ? config.localRoot
        : (vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0]
            ? vscode.workspace.workspaceFolders[0].uri.fsPath
            : '');
    this.mapper = new PathMapper(config.remoteRoot, localRoot);
    this.ws = null;
    this.outputChannel = vscode.window.createOutputChannel('FluffOS Debugger');
  }

  handleMessage(message) {
    if (!this.ws) {
      this._connect(message);
      return;
    }
    this._send(message);
  }

  _connect(firstMessage) {
    const { host, port } = this.config;
    this.outputChannel.appendLine(`Connecting to ${host}:${port} ...`);
    const sock = net.connect(port, host);
    sock.once('connect', () => {
      this.ws = new WSClient(sock, host, port);
      this.ws.onFrame = (payload) => {
        let msg;
        try {
          msg = JSON.parse(payload.toString('utf8'));
        } catch (e) {
          this.outputChannel.appendLine('Failed to parse message from driver: ' + e.message);
          return;
        }
        this.mapper.rewrite(msg, 'toLocal');
        this._onDidSendMessage.fire(msg);
      };
      this.ws.onClose = () => {
        this.outputChannel.appendLine('Driver closed the debugger connection.');
        this._onDidSendMessage.fire({ type: 'event', event: 'terminated', seq: 0 });
      };
      this.ws.onError = (e) => {
        this.outputChannel.appendLine('Debugger connection error: ' + e.message);
        vscode.window.showErrorMessage('FluffOS debugger: ' + e.message);
      };
      this._send(firstMessage);
    });
    sock.once('error', (e) => {
      this.outputChannel.appendLine('Connection failed: ' + e.message);
      vscode.window.showErrorMessage(`FluffOS debugger: could not connect to ${host}:${port} (${e.message})`);
      this._onDidSendMessage.fire({ type: 'event', event: 'terminated', seq: 0 });
    });
  }

  _send(message) {
    if (message.command === 'attach' && this.config.token) {
      message.arguments = message.arguments || {};
      if (message.arguments.token === undefined) {
        message.arguments.token = this.config.token;
      }
    }
    this.mapper.rewrite(message, 'toRemote');
    this.ws.sendText(JSON.stringify(message));
  }

  dispose() {
    if (this.ws) this.ws.close();
    this.outputChannel.dispose();
  }
}

class FluffosDebugAdapterDescriptorFactory {
  createDebugAdapterDescriptor(session) {
    return new vscode.DebugAdapterInlineImplementation(new FluffosDebugAdapter(session.configuration));
  }
}

class FluffosConfigurationProvider {
  resolveDebugConfiguration(_folder, config) {
    if (!config.type && !config.request) {
      // Launching with no launch.json entry at all (e.g. F5 with nothing
      // configured): hand back a starter attach configuration.
      config.type = 'fluffos';
      config.request = 'attach';
      config.name = 'Attach to FluffOS';
      config.host = '127.0.0.1';
      config.port = 4711;
    }
    if (!config.host) config.host = '127.0.0.1';
    if (!config.remoteRoot) config.remoteRoot = '/';
    if (!config.port) {
      vscode.window.showErrorMessage('FluffOS debugger: "port" is required (the driver\'s "debugger port").');
      return undefined;
    }
    return config;
  }
}

function activate(context) {
  context.subscriptions.push(
    vscode.debug.registerDebugConfigurationProvider('fluffos', new FluffosConfigurationProvider()),
    vscode.debug.registerDebugAdapterDescriptorFactory('fluffos', new FluffosDebugAdapterDescriptorFactory())
  );
}

function deactivate() {}

module.exports = { activate, deactivate, PathMapper };
