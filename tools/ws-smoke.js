#!/usr/bin/env node
// ws-smoke.js -- end-to-end websocket smoke test against the NATIVE driver.
//
//   node tools/ws-smoke.js [driver] [testsuite-dir]
//     driver        default build/src/driver (relative to the repo root)
//     testsuite-dir default testsuite/
//
// Boots `driver etc/config.test`, then exercises the websocket transport
// the way a real web client does -- something neither GTest nor the LPC
// suite touches (see src/www/AGENTS.md):
//
//   * HTTP GET on the ws port (the `websocket http dir` mount serves
//     src/www -- the shipped web client)
//   * a `telnet`-subprotocol connection driven through the REAL shared
//     browser telnet layer (src/www/telnet.js, eval'd into this script):
//     negotiation, the WILL/WONT SGA char-mode switch around get_char(),
//     and the /std/tui showcases
//   * a deterministic 5000-byte output burst (more than two 2048-byte lws
//     write windows) arriving completely -- a basic multi-window sanity
//     check. The size is generated via eval(), not read off a tuidemo
//     demo's actual output, so the check can't silently stop covering the
//     >1-window case if the demo content changes (tuidemo charts, for the
//     record, is only ~1.8KB -- comfortably under one window on its own)
//   * a genuine backpressure test: pause the client's socket, then push a
//     ~4.8MB burst -- enough to fill the kernel send buffer while the
//     "reader" can't drain it -- and confirm the connection recovers once
//     resumed. This is the actual regression test for the output wedge in
//     src/net/ws_telnet.cc / ws_ascii.cc: lws_send_pipe_choked() reports
//     the socket simply being full (zero-timeout poll) with no partial
//     write pending, and in that case lws never re-arms the writeable
//     callback on its own -- the drain loop must request it whenever it
//     exits with data still queued, or output freezes for good. The
//     quick 5000-byte burst above
//     does NOT exercise this -- on a fast, unpaused reader the whole
//     payload fits in the kernel's send buffer and no write ever blocks,
//     so it passes identically on fixed and unfixed code. Only forced
//     backpressure tells them apart.
//   * a live full-screen TUI (`tuidemo dashboard`): alternate screen,
//     sustained frame updates, clean quit
//   * a driver-initiated close while the connection is choked (destruct
//     the interactive mid-backpressure): teardown must release the
//     session resources even though the driver side is already gone --
//     an interim revision leaked a pending session timer here, a
//     use-after-free that aborted the driver on the ASan CI job
//   * an `ascii`-subprotocol connection with the same bursts, sent as
//     TEXT frames (ws_ascii.cc rejects binary frames outright) through
//     ws_ascii.cc's twin drain loop
//   * a TLS (`wss`) telnet connection: banner, then the same forced
//     backpressure through the TLS partial-write path
//
// No npm dependencies: the websocket client (handshake + framing) is
// implemented on net/tls sockets below. Exit code 0 = all checks passed.

'use strict';

const net = require('net');
const tls = require('tls');
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const http = require('http');
const { spawn } = require('child_process');

const repoRoot = path.resolve(__dirname, '..');
const driverPath = path.resolve(process.argv[2] || path.join(repoRoot, 'build/src/driver'));
const suiteDir = path.resolve(process.argv[3] || path.join(repoRoot, 'testsuite'));
const configRel = 'etc/config.test';

// The browser telnet client, shared verbatim with the web pages.
// (classic script, no module exports -- evaluate and take the binding)
const TelnetClient = eval(
  fs.readFileSync(path.join(repoRoot, 'src/www/telnet.js'), 'utf8') + '\n;TelnetClient');

// ---- config -----------------------------------------------------------

function wsPorts() {
  const conf = fs.readFileSync(path.join(suiteDir, configRel), 'utf8');
  const ports = [];
  for (const m of conf.matchAll(/^external_port_\d+\s*:\s*websocket\s+(\d+)/gm)) {
    ports.push(parseInt(m[1], 10));
  }
  if (ports.length < 1) throw new Error('no websocket ports in ' + configRel);
  return { plain: ports[0], tlsPort: ports[1] };  // config.test: 4001, 4002 (TLS)
}

// A deterministic multi-window burst: 5000 'x' bytes (more than two
// 2048-byte lws write windows) + a marker written last, so a truncated
// burst is unambiguous. Verified directly against both ws subprotocols
// (see the driver-side trace this test guards: a single writable
// callback draining 5024 bytes across 2048+2048+928-byte iterations).
const BURST_CMD = 'eval write(repeat_string("x", 5000) + "|END|"); return "done"';

// A ~4.8MB burst, looped because a single repeat_string() is capped by the
// "maximum string length" config (200000 in config.test) -- one call over
// that limit errors instead of producing a giant string, which silently
// degrades this into a no-op check (the error message is tiny and arrives
// instantly, so a paused/dead reader looks identical to a real freeze).
const BACKPRESSURE_BURST_CMD =
  'eval for (int i=0;i<600;i++) write(repeat_string("x", 8000)); write("|END|"); return "done"';

// ---- tiny websocket client (RFC6455 client side, no deps) ---------------

class WSClient {
  constructor(socket, host, port, subprotocol) {
    this.sock = socket;
    this.buf = Buffer.alloc(0);
    // Frames can arrive (and be parsed) before the caller has a chance to
    // assign onFrame -- the upgrade response and the first data frame(s)
    // often land in the same TCP segment on localhost. Queue anything that
    // shows up before a real handler is attached instead of dropping it
    // through a no-op default.
    this._onFrame = null;
    this._pending = [];
    this.onClose = () => {};
    this.established = false;
    this.negotiated = null;
    const key = crypto.randomBytes(16).toString('base64');
    this.acceptWant = crypto.createHash('sha1')
      .update(key + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11').digest('base64');
    socket.write(
      `GET / HTTP/1.1\r\nHost: ${host}:${port}\r\nUpgrade: websocket\r\n` +
      `Connection: Upgrade\r\nSec-WebSocket-Key: ${key}\r\n` +
      `Sec-WebSocket-Version: 13\r\nSec-WebSocket-Protocol: ${subprotocol}\r\n\r\n`);
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
      if (!/HTTP\/1\.1 101/.test(head)) throw new Error('upgrade refused:\n' + head);
      const accept = /sec-websocket-accept:\s*(\S+)/i.exec(head);
      if (!accept || accept[1] !== this.acceptWant) throw new Error('bad Sec-WebSocket-Accept');
      const proto = /sec-websocket-protocol:\s*(\S+)/i.exec(head);
      this.negotiated = proto ? proto[1] : null;
      this.established = true;
    }
    // frames
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
      if (b1 & 0x80) off += 4;  // masked server frame (never, but be safe)
      if (this.buf.length < off + len) return;
      const payload = this.buf.slice(off, off + len);
      this.buf = this.buf.slice(off + len);
      if (opcode === 0x9) { this.sendFrame(0xA, payload); continue; }  // ping -> pong
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
      throw new Error('frame too large for this test client');
    }
    const masked = Buffer.from(payload);
    for (let i = 0; i < masked.length; i++) masked[i] ^= mask[i & 3];
    this.sock.write(Buffer.concat([head, mask, masked]));
  }

  sendBinary(bytes) { this.sendFrame(0x2, Buffer.from(bytes)); }
  // ws_ascii.cc rejects binary frames outright (lws_frame_is_binary() ->
  // return -1, which closes the connection) -- the ascii subprotocol only
  // accepts TEXT frames from the client.
  sendText(str) { this.sendFrame(0x1, Buffer.from(str, 'utf8')); }
  close() { try { this.sock.destroy(); } catch (_) {} }
}

function connectWS(port, subprotocol, useTls) {
  return new Promise((resolve, reject) => {
    const to = setTimeout(() => reject(new Error('ws connect timeout')), 15000);
    const sock = useTls
      ? tls.connect({ port, host: '127.0.0.1', rejectUnauthorized: false }, onUp)
      : net.connect(port, '127.0.0.1', onUp);
    sock.on('error', (e) => { clearTimeout(to); reject(e); });
    let ws;
    function onUp() {
      ws = new WSClient(sock, '127.0.0.1', port, subprotocol);
      const poll = setInterval(() => {
        if (ws.established) {
          clearInterval(poll); clearTimeout(to);
          resolve(ws);
        }
      }, 20);
    }
  });
}

// ---- helpers ------------------------------------------------------------

const results = [];
function check(name, ok, detail) {
  results.push({ name, ok });
  console.log(`${ok ? 'PASS' : 'FAIL'}  ${name}${detail ? '  -- ' + detail : ''}`);
}

function waitFor(fn, timeout, what) {
  return new Promise((resolve) => {
    const t0 = Date.now();
    const iv = setInterval(() => {
      if (fn()) { clearInterval(iv); resolve(true); }
      else if (Date.now() - t0 > timeout) { clearInterval(iv); resolve(false); }
    }, 50);
  });
}

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

function httpGet(port, urlPath) {
  return new Promise((resolve, reject) => {
    http.get({ host: '127.0.0.1', port, path: urlPath }, (res) => {
      let body = '';
      res.on('data', (d) => { body += d; });
      res.on('end', () => resolve({ status: res.statusCode, body }));
    }).on('error', reject);
  });
}

// A telnet-subprotocol session: the shared TelnetClient does negotiation,
// we watch text/charmode and can push raw keystrokes.
function telnetSession(ws) {
  const s = { text: '', chunks: 0, charMode: false };
  const telnet = new TelnetClient((bytes) => ws.sendBinary(bytes));
  telnet.onText = (t) => { s.text += t; s.chunks++; };
  telnet.onCharMode = (on) => { s.charMode = on; };
  ws.onFrame = (payload) => telnet.receive(payload);
  s.sendText = (str) => telnet.sendRaw(Array.from(Buffer.from(str, 'utf8')));
  s.telnet = telnet;
  return s;
}

// ---- the test -----------------------------------------------------------

async function main() {
  const { plain, tlsPort } = wsPorts();

  if (!fs.existsSync(driverPath)) {
    console.error('driver not found: ' + driverPath);
    process.exit(2);
  }

  console.log(`booting ${driverPath} ${configRel} (ws ports: ${plain}${tlsPort ? ', tls ' + tlsPort : ''})`);
  const driver = spawn(driverPath, [configRel], { cwd: suiteDir, stdio: ['ignore', 'pipe', 'pipe'] });
  let driverLog = '';
  driver.stdout.on('data', (d) => { driverLog += d; });
  driver.stderr.on('data', (d) => { driverLog += d; });
  const kill = () => { try { driver.kill('SIGKILL'); } catch (_) {} };
  process.on('exit', kill);

  const up = await waitFor(() => driverLog.includes('Initializations complete'), 60000);
  if (!up) {
    console.error('driver did not boot; log tail:\n' + driverLog.slice(-2000));
    process.exit(2);
  }

  // 1. the ws port's HTTP mount serves the web client
  const page = await httpGet(plain, '/');
  check('http mount serves the web client', page.status === 200 && page.body.includes('xterm'),
        'status ' + page.status);
  const xt = await httpGet(plain, '/vendor/xterm.js');
  const tj = await httpGet(plain, '/telnet.js');
  check('http mount serves vendor/ and telnet.js', xt.status === 200 && tj.status === 200);

  // 2. telnet subprotocol: negotiation + banner through the REAL client
  const ws = await connectWS(plain, 'telnet', false);
  check('ws upgrade negotiates the telnet subprotocol', ws.negotiated === 'telnet');
  const s = telnetSession(ws);
  check('banner arrives', await waitFor(() => s.text.length > 50, 15000),
        s.text.length + ' chars');

  // 3. multi-window burst: basic sanity check that draining several
  //    2048-byte lws write windows in one callback works and delivers
  //    everything. A deterministic 5000-byte payload, via eval() so the
  //    size doesn't silently drift if the tuidemo demos change. This does
  //    NOT catch the lws wedge below -- see 3b.
  s.text = '';
  s.sendText(BURST_CMD + '\r\n');
  const gotBurst = await waitFor(() => s.text.includes('|END|'), 15000);
  check('multi-window burst arrives completely',
        gotBurst && s.text.length > 4096, s.text.length + ' chars');

  // 3b. genuine backpressure: pause the socket so the kernel send buffer
  //     actually fills, push ~4.8MB, then resume and confirm recovery --
  //     see BACKPRESSURE_BURST_CMD above for why the quick burst can't do
  //     this. Pre-fix, the connection stays wedged after resume forever.
  s.text = '';
  ws.sock.pause();
  s.sendText(BACKPRESSURE_BURST_CMD + '\r\n');
  await sleep(1500);
  ws.sock.resume();
  const gotBackpressureBurst = await waitFor(() => s.text.includes('|END|'), 20000);
  check('recovers from genuine backpressure (lws wedge regression)',
        gotBackpressureBurst && s.text.length > 4700000, s.text.length + ' chars');

  // 4. char mode: get_char() -> WILL SGA -> keystrokes -> quit -> WONT SGA
  s.sendText('tuidemo\r\n');
  check('readline arms char mode (WILL SGA)', await waitFor(() => s.charMode, 15000));
  for (const ch of 'quit\r') s.sendText(ch);
  check('quit returns to line mode (WONT SGA)', await waitFor(() => !s.charMode, 15000));

  // 5. live full-screen TUI: alternate screen + sustained frames + clean quit
  s.text = '';
  s.sendText('tuidemo dashboard\r\n');
  const altOn = await waitFor(() => s.text.includes('\x1b[?1049h'), 15000);
  check('dashboard enters the alternate screen', altOn);
  const chunksBefore = s.chunks;
  await sleep(1500);
  check('dashboard keeps streaming frames', s.chunks >= chunksBefore + 2,
        `${s.chunks - chunksBefore} updates in 1.5s`);
  s.sendText('q');
  check('q quits (alt screen restored)',
        await waitFor(() => s.text.includes('\x1b[?1049l'), 15000));
  await waitFor(() => !s.charMode, 15000);
  ws.close();

  // 5b. driver-initiated close while choked: destruct the interactive while
  //     its output is backpressured. LWS_CALLBACK_CLOSED must release the
  //     session resources even though pss->user is already nulled -- an
  //     interim revision leaked a pending session timer on this path, a
  //     use-after-free on the freed wsi/pss. On the ASan CI job such a bug
  //     aborts the driver deterministically (the follow-up connection below
  //     then fails); plain builds may survive it silently.
  const wsD = await connectWS(plain, 'telnet', false);
  const sd = telnetSession(wsD);
  await waitFor(() => sd.text.length > 50, 15000);
  wsD.sock.pause();
  sd.sendText(BACKPRESSURE_BURST_CMD + '\r\n');
  await sleep(1000);
  sd.sendText('eval destruct(this_player()); return "bye"\r\n');
  await sleep(2000);
  wsD.close();
  const wsE = await connectWS(plain, 'telnet', false);
  const se = telnetSession(wsE);
  check('driver survives destruct-while-choked (teardown UAF regression)',
        await waitFor(() => se.text.length > 50, 15000));
  wsE.close();

  // 6. ascii subprotocol: same big burst through ws_ascii.cc's drain loop.
  //    ws_ascii.cc rejects binary frames outright (return -1 on
  //    lws_frame_is_binary(), closing the connection) -- send TEXT frames.
  const wsA = await connectWS(plain, 'ascii', false);
  check('ascii subprotocol connects', wsA.negotiated === 'ascii');
  let asciiText = '';
  wsA.onFrame = (payload) => { asciiText += payload.toString('utf8'); };
  await waitFor(() => asciiText.length > 50, 15000);
  asciiText = '';
  wsA.sendText(BURST_CMD + '\n');
  const gotAscii = await waitFor(() => asciiText.includes('|END|'), 15000);
  check('ascii: multi-window burst arrives completely',
        gotAscii && asciiText.length > 4096, asciiText.length + ' chars');

  asciiText = '';
  wsA.sock.pause();
  wsA.sendText(BACKPRESSURE_BURST_CMD + '\n');
  await sleep(1500);
  wsA.sock.resume();
  const gotAsciiBackpressure = await waitFor(() => asciiText.includes('|END|'), 20000);
  check('ascii: recovers from genuine backpressure (lws wedge regression)',
        gotAsciiBackpressure && asciiText.length > 4700000, asciiText.length + ' chars');
  wsA.close();

  // 7. TLS websocket: banner, then forced backpressure through the TLS
  //    write path (SSL partial writes retry differently --
  //    LWS_SSL_CAPABLE_MORE_SERVICE -- than plain sockets, so exercise the
  //    choke/recovery machinery there too).
  if (tlsPort) {
    const wsT = await connectWS(tlsPort, 'telnet', true);
    const st = telnetSession(wsT);
    check('wss (TLS) telnet connection reaches the banner',
          await waitFor(() => st.text.length > 50, 15000));
    st.text = '';
    wsT.sock.pause();
    st.sendText(BACKPRESSURE_BURST_CMD + '\r\n');
    await sleep(1500);
    wsT.sock.resume();
    const gotTlsBp = await waitFor(() => st.text.includes('|END|'), 20000);
    check('wss: recovers from genuine backpressure',
          gotTlsBp && st.text.length > 4700000, st.text.length + ' chars');
    wsT.close();
  }

  kill();
  const failed = results.filter((r) => !r.ok);
  console.log(`\n${results.length - failed.length}/${results.length} websocket smoke checks passed`);
  process.exit(failed.length ? 1 : 0);
}

const watchdog = setTimeout(() => {
  console.error('ws-smoke: global timeout');
  process.exit(2);
}, 180000);
watchdog.unref();

main().catch((e) => {
  console.error('ws-smoke error:', e);
  process.exit(2);
});
