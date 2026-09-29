#!/usr/bin/env node
// ws-tls-reload.js -- prove sys_reload_tls() presents a replaced wss cert.
//
//   node tools/ws-tls-reload.js [driver] [testsuite-dir]
//
// Issue #1395: the efun used to refuse websocket ports, and later cuts
// "reloaded" the same files or mutated the live SSL_CTX in place while
// new handshakes still sent the boot leaf. This is the permanent gate:
// overwrite the on-disk cert/key with a freshly generated pair, call
// sys_reload_tls() on the live wss port, and require openssl s_client
// (and a Node TLS client) to see the new fingerprint. The original PEM
// files are restored on exit. GTest and the LPC suite cannot see the
// presented cert; they only check that the efun returns.
//
// Registered with ctest as `ws-tls-reload` (label `websocket`) so the
// CI "Run Unit Tests" step (`ctest -LE testsuite`) runs it. Also covered
// inside tools/ws-smoke.js on the Clang Debug job.

'use strict';

const net = require('net');
const tls = require('tls');
const crypto = require('crypto');
const fs = require('fs');
const os = require('os');
const path = require('path');
const { spawn, spawnSync, execFileSync } = require('child_process');

const repoRoot = path.resolve(__dirname, '..');
const driverPath = path.resolve(process.argv[2] || path.join(repoRoot, 'build/src/driver'));
const suiteDir = path.resolve(process.argv[3] || path.join(repoRoot, 'testsuite'));
const configRel = 'etc/config.test';

function wsTlsPort() {
  const conf = fs.readFileSync(path.join(suiteDir, configRel), 'utf8');
  let plain;
  let tlsIndex = 0;
  let tlsPort;
  for (const m of conf.matchAll(/^external_port_(\d+)\s*:\s*websocket\s+(\d+)/gm)) {
    const index = parseInt(m[1], 10);
    const port = parseInt(m[2], 10);
    if (plain === undefined) plain = port;
    if (new RegExp('^external_port_' + index + '_tls\\s*:', 'm').test(conf)) {
      tlsIndex = index;
      tlsPort = port;
    }
  }
  if (!plain || !tlsPort) {
    throw new Error('config.test needs a plain websocket port and a TLS websocket port');
  }
  return { plain, tlsPort, tlsIndex };
}

function tlsFilePaths() {
  const conf = fs.readFileSync(path.join(suiteDir, configRel), 'utf8');
  const m = conf.match(/^external_port_\d+_tls\s*:\s*cert=(\S+)\s+key=(\S+)/m);
  if (!m) throw new Error('no external_port_N_tls cert=/key= in ' + configRel);
  return {
    cert: path.resolve(suiteDir, m[1]),
    key: path.resolve(suiteDir, m[2]),
  };
}

function opensslPeerCert(port) {
  const sclient = spawnSync('openssl', [
    's_client', '-connect', `127.0.0.1:${port}`,
  ], { input: '', encoding: 'utf8', maxBuffer: 1024 * 1024 });
  const x509 = spawnSync('openssl', [
    'x509', '-noout', '-subject', '-fingerprint', '-sha256',
  ], { input: sclient.stdout, encoding: 'utf8' });
  if (x509.status !== 0 || !x509.stdout.trim()) {
    throw new Error(
      'openssl could not read the peer cert on port ' + port + ': ' +
      ((x509.stderr || sclient.stderr || '').toString().slice(-400)));
  }
  return x509.stdout.trim();
}

function opensslFileCert(certPath) {
  const x509 = spawnSync('openssl', [
    'x509', '-in', certPath, '-noout', '-subject', '-fingerprint', '-sha256',
  ], { encoding: 'utf8' });
  if (x509.status !== 0 || !x509.stdout.trim()) {
    throw new Error('openssl could not read ' + certPath);
  }
  return x509.stdout.trim();
}

function writeFreshSelfSigned(certPath, keyPath) {
  const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'fluffos-ws-cert-'));
  const newCert = path.join(tmp, 'cert.pem');
  const newKey = path.join(tmp, 'key.pem');
  execFileSync('openssl', [
    'req', '-x509', '-newkey', 'rsa:2048', '-sha256',
    '-keyout', newKey, '-out', newCert,
    '-days', '2', '-nodes',
    '-subj', '/CN=fluffos-ws-reload-1395',
  ], { stdio: 'pipe' });
  fs.copyFileSync(newCert, certPath);
  fs.copyFileSync(newKey, keyPath);
  fs.rmSync(tmp, { recursive: true, force: true });
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

function evalOnPlainWs(port, cmd, timeoutMs) {
  return new Promise((resolve, reject) => {
    const key = crypto.randomBytes(16).toString('base64');
    const sock = net.connect(port, '127.0.0.1');
    let buf = Buffer.alloc(0);
    let up = false;
    const to = setTimeout(() => {
      sock.destroy();
      reject(new Error('ws eval timeout; got ' + buf.toString('utf8').slice(-200)));
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
        sendTextFrame(sock, cmd);
      }
      if (up && buf.toString('utf8').includes('|RELOADED|')) {
        clearTimeout(to);
        sock.destroy();
        resolve();
      }
    });
  });
}

function nodePeerFingerprint(port) {
  return new Promise((resolve, reject) => {
    const to = setTimeout(() => reject(new Error('tls connect timeout')), 10000);
    const sock = tls.connect({
      port,
      host: '127.0.0.1',
      rejectUnauthorized: false,
      maxCachedSessions: 0,
    }, () => {
      const cert = sock.getPeerCertificate();
      const fp = cert && (cert.fingerprint256 || cert.fingerprint);
      sock.destroy();
      clearTimeout(to);
      if (!fp) reject(new Error('no peer certificate'));
      else resolve(fp);
    });
    sock.on('error', (e) => { clearTimeout(to); reject(e); });
  });
}

function fail(name, detail) {
  console.log(`[  FAILED  ] ${name} -- ${detail}`);
  return 1;
}

async function main() {
  const { plain, tlsPort, tlsIndex } = wsTlsPort();
  const tlsFiles = tlsFilePaths();

  if (!fs.existsSync(driverPath)) {
    console.error('driver not found: ' + driverPath);
    process.exit(2);
  }

  console.log(`[ RUN      ] ws-tls-reload.presents_new_cert`);
  const origCert = fs.readFileSync(tlsFiles.cert);
  const origKey = fs.readFileSync(tlsFiles.key);
  const driver = spawn(driverPath, [configRel], {
    cwd: suiteDir, stdio: ['ignore', 'pipe', 'pipe'],
  });
  let log = '';
  driver.stdout.on('data', (d) => { log += d; });
  driver.stderr.on('data', (d) => { log += d; });
  const cleanup = () => {
    try { fs.writeFileSync(tlsFiles.cert, origCert); } catch (_) {}
    try { fs.writeFileSync(tlsFiles.key, origKey); } catch (_) {}
    try { driver.kill('SIGKILL'); } catch (_) {}
  };
  process.on('exit', cleanup);

  const t0 = Date.now();
  while (!log.includes('Initializations complete')) {
    if (Date.now() - t0 > 60000) {
      cleanup();
      process.exit(fail('ws-tls-reload.presents_new_cert', 'driver did not boot'));
    }
    await new Promise((r) => setTimeout(r, 50));
  }

  const before = opensslPeerCert(tlsPort);
  const nodeBefore = await nodePeerFingerprint(tlsPort);
  writeFreshSelfSigned(tlsFiles.cert, tlsFiles.key);
  const onDisk = opensslFileCert(tlsFiles.cert);
  if (onDisk === before) {
    cleanup();
    process.exit(fail('ws-tls-reload.presents_new_cert',
      'generated replacement cert matches the boot cert'));
  }

  await evalOnPlainWs(
    plain,
    `eval sys_reload_tls(${tlsIndex}); write("|RELOADED|"); return 0\r\n`,
    15000);
  if (!log.includes('Reloading TLS config for port ' + tlsPort)) {
    cleanup();
    process.exit(fail('ws-tls-reload.presents_new_cert',
      'driver did not log the reload'));
  }

  const after = opensslPeerCert(tlsPort);
  const nodeAfter = await nodePeerFingerprint(tlsPort);
  if (after === before) {
    cleanup();
    process.exit(fail('ws-tls-reload.presents_new_cert',
      'openssl s_client still sees the boot cert:\n' + after));
  }
  if (after !== onDisk) {
    cleanup();
    process.exit(fail('ws-tls-reload.presents_new_cert',
      'presented cert does not match the on-disk replacement:\n' +
      'disk ' + onDisk + '\npeer ' + after));
  }
  if (!nodeAfter || nodeAfter === nodeBefore) {
    cleanup();
    process.exit(fail('ws-tls-reload.presents_new_cert',
      'Node TLS client still sees the boot fingerprint ' + nodeBefore));
  }

  console.log(`[       OK ] ws-tls-reload.presents_new_cert`);
  console.log(`            before ${before.replace(/\n/g, ' | ')}`);
  console.log(`            after  ${after.replace(/\n/g, ' | ')}`);
  cleanup();
  process.exit(0);
}

const watchdog = setTimeout(() => {
  console.error('ws-tls-reload: global timeout');
  process.exit(2);
}, 90000);
watchdog.unref();

main().catch((e) => {
  console.error(e);
  process.exit(2);
});
