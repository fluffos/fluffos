#!/usr/bin/env node
// e2e-live.js -- end-to-end checks that need a LIVE driver event loop.
//
//   node tools/e2e-live.js [driver] [testsuite-dir]
//
// The LPC suite runs inside master::flag(), where nothing is serviced: no
// network events, no async completions. Two fixes could therefore only be
// proven here, by booting the driver normally, driving it over the plain
// websocket port with `eval`, and talking to it from outside:
//
//   utf8_split_reads  (#1422)  a multi-byte UTF-8 character split across TCP
//                              reads reaches a text-mode STREAM socket's read
//                              callback intact, not as U+FFFD fragments.
//   async_nested_callbacks (#1421)  async_read callbacks that queue further
//                              async_reads must not freeze the main thread
//                              (finished_reqs_lock held across LPC).
//
// Registered with ctest as `e2e-live` (label `e2e`), so the CI unit-test step
// (`ctest -LE testsuite`) runs it. Needs only node.

'use strict';

const net = require('net');
const crypto = require('crypto');
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');

const repoRoot = path.resolve(__dirname, '..');
const driverPath = path.resolve(process.argv[2] || path.join(repoRoot, 'build/src/driver'));
const suiteDir = path.resolve(process.argv[3] || path.join(repoRoot, 'testsuite'));
const configRel = 'etc/config.test';

function plainWsPort() {
  const conf = fs.readFileSync(path.join(suiteDir, configRel), 'utf8');
  const m = conf.match(/^external_port_\d+\s*:\s*websocket\s+(\d+)/m);
  if (!m) throw new Error('config.test needs a plain websocket port');
  return parseInt(m[1], 10);
}

function sendTextFrame(sock, text) {
  const payload = Buffer.from(text);
  const mask = crypto.randomBytes(4);
  let head;
  if (payload.length < 126) {
    head = Buffer.from([0x81, 0x80 | payload.length]);
  } else {
    head = Buffer.alloc(4);
    head[0] = 0x81;
    head[1] = 0x80 | 126;
    head.writeUInt16BE(payload.length, 2);
  }
  const masked = Buffer.from(payload);
  for (let i = 0; i < masked.length; i++) masked[i] ^= mask[i & 3];
  sock.write(Buffer.concat([head, mask, masked]));
}

// Run `eval <code>` on a fresh websocket login; resolve with everything the
// driver wrote once `marker` shows up (a frozen driver never answers).
function evalWs(port, code, marker, timeoutMs) {
  return new Promise((resolve, reject) => {
    const key = crypto.randomBytes(16).toString('base64');
    const sock = net.connect(port, '127.0.0.1');
    let buf = Buffer.alloc(0);
    let up = false;
    const to = setTimeout(() => {
      sock.destroy();
      reject(new Error('eval timed out waiting for ' + marker + '; got ' +
                       JSON.stringify(buf.toString('utf8').slice(-200))));
    }, timeoutMs);
    sock.on('error', (e) => { clearTimeout(to); reject(e); });
    sock.on('connect', () => {
      sock.write(
        `GET / HTTP/1.1\r\nHost: 127.0.0.1:${port}\r\nUpgrade: websocket\r\n` +
        'Connection: Upgrade\r\nSec-WebSocket-Key: ' + key + '\r\n' +
        'Sec-WebSocket-Version: 13\r\nSec-WebSocket-Protocol: telnet\r\n\r\n');
    });
    sock.on('data', (d) => {
      buf = Buffer.concat([buf, d]);
      if (!up) {
        const i = buf.indexOf('\r\n\r\n');
        if (i < 0) return;
        const head = buf.slice(0, i).toString('latin1');
        if (!/HTTP\/1\.1 101/.test(head)) {
          clearTimeout(to);
          sock.destroy();
          return reject(new Error('upgrade refused: ' + head.slice(0, 200)));
        }
        up = true;
        buf = buf.slice(i + 4);
        sendTextFrame(sock, code + '\r\n');
      }
      const text = buf.toString('utf8');
      if (up && text.includes(marker)) {
        clearTimeout(to);
        sock.destroy();
        resolve(text);
      }
    });
  });
}

function freePort() {
  return new Promise((resolve, reject) => {
    const srv = net.createServer();
    srv.on('error', reject);
    srv.listen(0, '127.0.0.1', () => {
      const { port } = srv.address();
      srv.close(() => resolve(port));
    });
  });
}

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

// Send `chunks` one TCP segment at a time and collect the echo until it is
// as long as what was sent (or the wait expires).
function echoRoundTrip(port, chunks, gapMs, waitMs) {
  return new Promise((resolve, reject) => {
    const sent = Buffer.concat(chunks);
    const sock = net.connect(port, '127.0.0.1');
    sock.setNoDelay(true);
    let got = Buffer.alloc(0);
    const finish = () => { clearTimeout(to); sock.destroy(); resolve({ sent, got }); };
    const to = setTimeout(finish, waitMs);
    sock.on('error', (e) => { clearTimeout(to); reject(e); });
    sock.on('data', (d) => {
      got = Buffer.concat([got, d]);
      if (got.length >= sent.length) finish();
    });
    sock.on('connect', async () => {
      for (const c of chunks) {
        sock.write(c);
        await sleep(gapMs);
      }
    });
  });
}

const results = [];
function record(name, ok, detail) {
  results.push(ok);
  if (ok) {
    console.log(`[       OK ] e2e-live.${name}`);
  } else {
    console.log(`[  FAILED  ] e2e-live.${name} -- ${detail}`);
  }
}

async function testUtf8SplitReads(wsPort) {
  const name = 'utf8_split_reads';
  console.log(`[ RUN      ] e2e-live.${name}`);
  const echoPort = await freePort();
  const out = await evalWs(
    wsPort,
    `eval object o = new("/clone/e2e_utf8_echo"); write("|STARTED=" + o->start(${echoPort}) + "|"); return 0`,
    '|STARTED=', 15000);
  if (out.includes('|STARTED=-999|')) {
    console.log(`[  SKIPPED ] e2e-live.${name} -- driver built without PACKAGE_SOCKETS`);
    return record(name, true);
  }
  if (!out.includes('|STARTED=1|')) {
    return record(name, false, 'echo server did not start: ' + JSON.stringify(out.slice(-200)));
  }

  const B = (...bytes) => Buffer.from(bytes);
  const cases = [
    ['ascii passthrough', [Buffer.from('hello\n')]],
    ['3-byte char split 2+1', [Buffer.from('ab'), B(0xe4, 0xbd), B(0xa0), Buffer.from('cd')]],
    ['two CJK chars split mid-sequence', [B(0xe4, 0xbd), B(0xa0, 0xe5, 0xa5), B(0xbd)]],
    ['4-byte emoji one byte per write',
      [Buffer.from('x'), B(0xf0), B(0x9f), B(0x98), B(0x80), Buffer.from('y')]],
  ];
  for (const [label, chunks] of cases) {
    const { sent, got } = await echoRoundTrip(echoPort, chunks, 150, 6000);
    if (!got.equals(sent)) {
      return record(name, false,
        `${label}: sent ${sent.toString('hex')} but LPC echoed ${got.toString('hex')}`);
    }
  }
  record(name, true);
}

async function testAsyncNestedCallbacks(wsPort) {
  const name = 'async_nested_callbacks';
  console.log(`[ RUN      ] e2e-live.${name}`);
  const total = 6000;
  const out = await evalWs(
    wsPort,
    `eval object o = load_object("/clone/e2e_async_nested"); o->start(8, ${total}); write("|QUEUED|"); return 0`,
    '|QUEUED|', 15000).catch((e) => {
    throw new Error('driver stopped answering after nested async_reads were queued ' +
                    '(main-thread deadlock in check_reqs?): ' + e.message.slice(0, 120));
  });
  if (!out.includes('|QUEUED|')) return record(name, false, 'could not queue');

  // Each poll is itself an eval on the main thread: a frozen loop never
  // answers, so evalWs's timeout is the deadlock detector.
  const deadline = Date.now() + 60000;
  let status = '';
  for (;;) {
    const t = await evalWs(
      wsPort,
      'eval write(load_object("/clone/e2e_async_nested")->status()); return 0',
      'BAD=', 15000).catch((e) => { throw new Error('driver stopped answering (deadlock?): ' + e.message); });
    const m = t.match(/\|DONE=(\d+)\/(\d+) BAD=(\d+)\|/);
    if (m) {
      status = m[0];
      if (parseInt(m[3], 10) !== 0) {
        return record(name, false, 'async_read callback got a non-string result: ' + status);
      }
      if (parseInt(m[1], 10) >= total) break;
    }
    if (Date.now() > deadline) {
      return record(name, false, 'chains did not finish: ' + status);
    }
    await sleep(200);
  }
  record(name, true);
}

async function main() {
  if (!fs.existsSync(driverPath)) {
    console.error('driver not found: ' + driverPath);
    process.exit(2);
  }
  const wsPort = plainWsPort();
  const driver = spawn(driverPath, [configRel], {
    cwd: suiteDir, stdio: ['ignore', 'pipe', 'pipe'],
  });
  let log = '';
  driver.stdout.on('data', (d) => { log += d; });
  driver.stderr.on('data', (d) => { log += d; });
  const cleanup = () => {
    try { driver.kill('SIGKILL'); } catch (_) {}
    // `eval` writes this scratch file into the mudlib; leaving it makes
    // testsuite/format.sh --check flag it.
    try { fs.rmSync(path.join(suiteDir, 'tmp_eval_file.c'), { force: true }); } catch (_) {}
  };
  process.on('exit', cleanup);

  const t0 = Date.now();
  while (!log.includes('Initializations complete')) {
    if (Date.now() - t0 > 60000) {
      console.log('[  FAILED  ] e2e-live -- driver did not boot\n' + log.slice(-500));
      process.exit(1);
    }
    await sleep(50);
  }

  const only = process.env.E2E_ONLY;
  try {
    if (!only || only === 'utf8') await testUtf8SplitReads(wsPort);
    if (!only || only === 'async') await testAsyncNestedCallbacks(wsPort);
  } catch (e) {
    record('harness', false, e.message);
  }
  const failed = results.filter((ok) => !ok).length;
  if (failed) console.log('--- driver log tail ---\n' + log.slice(-1500));
  process.exit(failed ? 1 : 0);
}

const watchdog = setTimeout(() => {
  console.error('e2e-live: global timeout');
  process.exit(2);
}, 180000);
watchdog.unref();

main().catch((e) => {
  console.error(e);
  process.exit(2);
});
