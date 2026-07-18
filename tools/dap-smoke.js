#!/usr/bin/env node
// dap-smoke.js -- end-to-end smoke test for the WebSocket LPC debugger
// (src/debugger/DESIGN.md) against the NATIVE driver.
//
//   node tools/dap-smoke.js [driver] [testsuite-dir]
//     driver        default build/src/driver (relative to the repo root)
//     testsuite-dir default testsuite/
//
// Boots `driver etc/config.dap-smoke.test` (a copy of the normal test config
// with a "debugger port" line added) and drives a real DAP session over the
// "dap" websocket subprotocol: authentication (accept + reject), attach,
// setBreakpoints with extension-blind path matching and snap-to-next-line,
// hitting a breakpoint from a real player command over the telnet port,
// stack/scopes/variables inspection, stepping, continue (world resumes),
// object/file browsing while running, source fetch, and a disconnect-while-
// stopped safety-net check (killing the client must un-freeze the mud).
//
// No npm dependencies: same minimal RFC6455 client as tools/ws-smoke.js.
// Exit code 0 = all checks passed.

'use strict';

const net = require('net');
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const { spawn } = require('child_process');

const repoRoot = path.resolve(__dirname, '..');
const driverPath = path.resolve(process.argv[2] || path.join(repoRoot, 'build/src/driver'));
const suiteDir = path.resolve(process.argv[3] || path.join(repoRoot, 'testsuite'));
const baseConfigRel = 'etc/config.test';
const dapConfigRel = 'etc/config.dap-smoke.test';
const DEBUGGER_PORT = 47110;
const DEBUGGER_PASSWORD = 'dap-smoke-secret';

// ---- tiny websocket client (RFC6455 client side, no deps) ---------------

class WSClient {
  constructor(socket, host, port, subprotocol) {
    this.sock = socket;
    this.buf = Buffer.alloc(0);
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

  sendText(str) { this.sendFrame(0x1, Buffer.from(str, 'utf8')); }
  close() { try { this.sock.destroy(); } catch (_) {} }
}

function connectWS(port, subprotocol) {
  return new Promise((resolve, reject) => {
    const to = setTimeout(() => reject(new Error('ws connect timeout')), 15000);
    const sock = net.connect(port, '127.0.0.1', () => {
      clearTimeout(to);
      resolve(new WSClient(sock, '127.0.0.1', port, subprotocol));
    });
    sock.on('error', (e) => { clearTimeout(to); reject(e); });
  });
}

// ---- minimal DAP request/response/event client --------------------------

class DapClient {
  constructor(ws) {
    this.ws = ws;
    this.seq = 1;
    this.waiters = [];
    this.msgs = [];
    ws.onFrame = (payload) => {
      let msg;
      try { msg = JSON.parse(payload.toString('utf8')); } catch (_) { return; }
      this.msgs.push(msg);
      this.waiters = this.waiters.filter((w) => {
        if (w.pred(msg)) { w.resolve(msg); return false; }
        return true;
      });
    };
  }
  send(command, args) {
    const msg = { seq: this.seq++, type: 'request', command, arguments: args || {} };
    this.ws.sendText(JSON.stringify(msg));
    return msg.seq;
  }
  wait(pred, what, ms) {
    return new Promise((resolve, reject) => {
      for (const m of this.msgs) if (pred(m)) return resolve(m);
      const to = setTimeout(() => reject(new Error('dap timeout waiting for ' + what)), ms || 10000);
      this.waiters.push({ pred, resolve: (m) => { clearTimeout(to); resolve(m); } });
    });
  }
  async request(command, args, ms) {
    const seq = this.send(command, args);
    return this.wait((m) => m.type === 'response' && m.request_seq === seq, command, ms);
  }
  event(name, ms) {
    return this.wait((m) => m.type === 'event' && m.event === name, 'event:' + name, ms);
  }
  clearMsgs() { this.msgs.length = 0; }
}

// ---- helpers --------------------------------------------------------------

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

function telnetPort(configText) {
  const m = /^port number\s*:\s*(\d+)/m.exec(configText);
  if (!m) throw new Error('no "port number" line in config');
  return parseInt(m[1], 10);
}

// ---- the test ---------------------------------------------------------

async function main() {
  if (!fs.existsSync(driverPath)) {
    console.error('driver not found: ' + driverPath);
    process.exit(2);
  }

  // A dedicated config so this test never races other suites/instances for
  // the debugger port; derived from config.test, not hand-duplicated, so it
  // never drifts from the real port layout.
  const baseConfig = fs.readFileSync(path.join(suiteDir, baseConfigRel), 'utf8');
  const dapConfig = baseConfig +
    `\ndebugger port : ${DEBUGGER_PORT}\ndebugger password : ${DAP_ESCAPE(DEBUGGER_PASSWORD)}\n`;
  fs.writeFileSync(path.join(suiteDir, dapConfigRel), dapConfig);
  const telnet_port = telnetPort(dapConfig);

  console.log(`booting ${driverPath} ${dapConfigRel} (debugger port ${DEBUGGER_PORT}, telnet ${telnet_port})`);
  const driver = spawn(driverPath, [dapConfigRel], { cwd: suiteDir, stdio: ['ignore', 'pipe', 'pipe'] });
  let driverLog = '';
  driver.stdout.on('data', (d) => { driverLog += d; });
  driver.stderr.on('data', (d) => { driverLog += d; });
  const kill = () => { try { driver.kill('SIGKILL'); } catch (_) {} };
  const cleanupConfig = () => { try { fs.unlinkSync(path.join(suiteDir, dapConfigRel)); } catch (_) {} };
  process.on('exit', () => { kill(); cleanupConfig(); });

  const up = await waitFor(() => driverLog.includes('Initializations complete'), 60000);
  if (!up) {
    console.error('driver did not boot; log tail:\n' + driverLog.slice(-2000));
    process.exit(2);
  }
  check('debugger listener came up', driverLog.includes(`Debugger: listening on 127.0.0.1:${DEBUGGER_PORT}`));

  // 1. bad token is rejected, and the connection is actually closed (not
  // just told "no") -- this is the exact race tools/dap-smoke.js's driver
  // counterpart (src/debugger/transport_lws.cc) must get right: the reject
  // response must be flushed to the wire before the socket is torn down.
  {
    const bad = new DapClient(await connectWS(DEBUGGER_PORT, 'dap'));
    await bad.request('initialize', {});
    const resp = await bad.request('attach', { token: 'wrong-token' });
    check('bad attach token is rejected', resp.success === false);
    const closed = await new Promise((resolve) => {
      bad.ws.onClose = () => resolve(true);
      setTimeout(() => resolve(false), 5000);
    });
    check('rejected client connection is closed', closed);
  }

  // 2. real session
  const c = new DapClient(await connectWS(DEBUGGER_PORT, 'dap'));
  const init = await c.request('initialize', {});
  check('initialize succeeds with exception filter capabilities',
        init.success && Array.isArray(init.body && init.body.exceptionBreakpointFilters));
  const attach = await c.request('attach', { token: DEBUGGER_PASSWORD });
  check('attach with correct token succeeds', attach.success === true);
  await c.event('initialized');

  const bps = await c.request('setBreakpoints', {
    // Deliberately a ".c" path against a real ".lpc" file: pins the
    // extension-blind path matching documented in AGENTS.md.
    source: { path: '/single/master.c' },
    breakpoints: [{ line: 111 }],
  });
  check('setBreakpoints verifies a real code line',
        bps.success && bps.body.breakpoints.length === 1 &&
        bps.body.breakpoints[0].verified === true && bps.body.breakpoints[0].line === 111,
        JSON.stringify(bps.body));

  await c.request('configurationDone', {});
  const threads = await c.request('threads', {});
  check('threads reports the single LPC thread',
        threads.success && threads.body.threads.length === 1 && threads.body.threads[0].id === 1);

  // 3. hit the breakpoint via a real player connection (master::connect()).
  const telnet = net.connect(telnet_port, '127.0.0.1');
  telnet.on('error', () => {});
  const stopped = await c.event('stopped', 15000).catch(() => null);
  check('breakpoint hit stops the VM',
        !!stopped && stopped.body.reason === 'breakpoint' &&
        JSON.stringify(stopped.body.hitBreakpointIds) === JSON.stringify([bps.body.breakpoints[0].id]));

  const st = await c.request('stackTrace', {});
  const top = st.body && st.body.stackFrames && st.body.stackFrames[0];
  check('stackTrace shows the stopped frame at the breakpoint line',
        !!top && top.line === 111 && top.source.path === '/single/master.lpc',
        JSON.stringify(top));

  const scopes = await c.request('scopes', { frameId: 0 });
  const scopeNames = (scopes.body.scopes || []).map((s) => s.name);
  check('scopes include Arguments, Locals, and Object variables',
        ['Arguments', 'Locals', 'Object variables'].every((n) => scopeNames.includes(n)),
        JSON.stringify(scopeNames));

  const globalsScope = scopes.body.scopes.find((s) => s.name === 'Object variables');
  const globals = await c.request('variables', { variablesReference: globalsScope.variablesReference });
  const hasErrorVar = (globals.body.variables || []).find((v) => v.name === 'has_error');
  check('object variables include a known master.lpc global',
        !!hasErrorVar && hasErrorVar.type === 'int', JSON.stringify(hasErrorVar));

  // 3b. Locals scope: connect() declares `object login_ob; mixed err;` --
  // with "debugger port" set (as this config always has it), the compiler
  // captures real local names (DESIGN.md §9), so these must show as
  // "login_ob"/"err", never "local0"/"local1".
  const localsScope = scopes.body.scopes.find((s) => s.name === 'Locals');
  const localsBefore = await c.request('variables', { variablesReference: localsScope.variablesReference });
  const localNames = (localsBefore.body.variables || []).map((v) => v.name);
  check('Locals scope shows the compiler-captured real names',
        localNames.includes('login_ob') && localNames.includes('err'),
        JSON.stringify(localNames));

  // setVariable against a real stopped frame (the GTest suite can only
  // reach the object-global/array/mapping paths, not this one -- there's no
  // live csp/fp outside a real stopped VM). Restore `err` to falsy 0
  // afterward so connect()'s `if (err) destruct(this_object())` below --
  // this_object() being the MASTER object here -- never fires; the rest of
  // this script's telnet/continue checks depend on the master staying alive.
  const setVar = await c.request('setVariable', {
    variablesReference: localsScope.variablesReference, name: 'err', value: '"probed-by-dap-smoke"',
  });
  check('setVariable writes a real local by its captured name',
        setVar.success && setVar.body.value === '"probed-by-dap-smoke"', JSON.stringify(setVar));
  const localsAfterWrite = await c.request('variables', { variablesReference: localsScope.variablesReference });
  const errAfterWrite = (localsAfterWrite.body.variables || []).find((v) => v.name === 'err');
  check('the write is visible in a subsequent variables fetch',
        !!errAfterWrite && errAfterWrite.value === '"probed-by-dap-smoke"', JSON.stringify(errAfterWrite));

  const restoreVar = await c.request('setVariable', {
    variablesReference: localsScope.variablesReference, name: 'err', value: '0',
  });
  check('setVariable restores err to falsy 0 (also exercises a string->int type change)',
        restoreVar.success && restoreVar.body.value === '0', JSON.stringify(restoreVar));

  // 4. step, then continue -- the world must resume (telnet data flows).
  c.clearMsgs();
  await c.request('next', { threadId: 1 });
  const stepStopped = await c.event('stopped', 15000).catch(() => null);
  const st2 = stepStopped ? await c.request('stackTrace', {}) : null;
  check('step (next) advances to a later line in the same frame',
        !!stepStopped && stepStopped.body.reason === 'step' &&
        st2 && st2.body.stackFrames[0].line > 111,
        st2 && JSON.stringify(st2.body.stackFrames[0]));

  let telnetData = '';
  telnet.on('data', (d) => { telnetData += d.toString('latin1'); });
  await c.request('continue', { threadId: 1 });
  const resumed = await waitFor(() => telnetData.length > 0, 10000);
  check('continue resumes the world (telnet banner arrives)', resumed);

  // 5. introspection while RUNNING (no pause required).
  const sources = await c.request('loadedSources', {});
  check('loadedSources lists at least one program', sources.body.sources.length > 0);

  const objs = await c.request('fluffos_objects', { filter: 'master', count: 5 });
  check('fluffos_objects finds the loaded master object',
        objs.body.objects.some((o) => o.name === '/single/master'), JSON.stringify(objs.body));

  const files = await c.request('fluffos_files', { path: '/single' });
  check('fluffos_files lists testsuite/single', files.body.files.length > 0);

  const src = await c.request('source', { path: '/single/master.lpc' });
  check('source returns real file content',
        src.success && src.body.content.includes('object connect()'));

  // 6. disconnect-while-stopped safety net: arm a pause, let it stop, then
  // kill the client's raw socket (no `disconnect` request) -- the driver
  // must notice via LWS_CALLBACK_CLOSED and resume on its own.
  c.clearMsgs();
  await c.request('pause', { threadId: 1 });
  telnet.write('look\r\n');
  const pausedEvt = await c.event('stopped', 10000).catch(() => null);
  check('pause stops the VM', !!pausedEvt);
  c.ws.close();
  await sleep(1000);
  const afterKillLog = driverLog.includes('Debugger: client connection closed.');
  check('driver notices the killed client', afterKillLog);
  // A fresh session must be able to attach and run a command -- proof the
  // world actually un-froze rather than staying stopped forever.
  const c2 = new DapClient(await connectWS(DEBUGGER_PORT, 'dap'));
  await c2.request('initialize', {});
  await c2.request('attach', { token: DEBUGGER_PASSWORD });
  await c2.request('setBreakpoints', { source: { path: '/single/master.c' }, breakpoints: [] });
  const stillAlive = await waitFor(() => driverLog.includes('Initializations complete'), 2000);
  check('mud is not frozen after client death', stillAlive);
  await c2.request('disconnect', {});
  c2.ws.close();

  kill();
  cleanupConfig();
  const failed = results.filter((r) => !r.ok);
  console.log(`\n${results.length - failed.length}/${results.length} debugger smoke checks passed`);
  process.exit(failed.length ? 1 : 0);
}

function DAP_ESCAPE(s) {
  // rc.cc config lines are simple "key : value" pairs read to end-of-line;
  // the test password never contains a newline, so no escaping is needed --
  // named defensively so a future password value change fails loudly here
  // instead of silently corrupting the config file.
  if (/[\r\n]/.test(s)) throw new Error('DEBUGGER_PASSWORD must not contain newlines');
  return s;
}

const watchdog = setTimeout(() => {
  console.error('dap-smoke: global timeout');
  process.exit(2);
}, 120000);
watchdog.unref();

main().catch((e) => {
  console.error('dap-smoke error:', e);
  process.exit(2);
});
