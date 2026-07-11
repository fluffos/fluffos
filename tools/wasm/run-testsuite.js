#!/usr/bin/env node
// Run the LPC testsuite inside the WASM driver under node — the
// equivalent of `driver etc/config.test -ftest` for the web build.
//
// Usage: node tools/wasm/run-testsuite.js [build-wasm/src] [testsuite]
//
// Exits 0 when the driver's test run exits cleanly, nonzero otherwise
// (same contract as the native ctest `testsuite` entry).

const fs = require('fs');
const path = require('path');

const buildDir = path.resolve(process.argv[2] || path.join(__dirname, '../../build-wasm/src'));
const suiteDir = path.resolve(process.argv[3] || path.join(__dirname, '../../testsuite'));

const createFluffOS = require(path.join(buildDir, 'fluffos.js'));

function copyDir(Module, src, dst) {
  try { Module.FS.mkdir(dst); } catch (e) { /* exists */ }
  for (const e of fs.readdirSync(src, { withFileTypes: true })) {
    const s = path.join(src, e.name);
    const d = dst + '/' + e.name;
    if (e.isDirectory()) copyDir(Module, s, d);
    else if (e.isFile()) Module.FS.writeFile(d, fs.readFileSync(s));
  }
}

(async () => {
  const Module = await createFluffOS({
    // Old emscripten glue prefers fetch() under node 18+; hand it the
    // binary directly instead.
    wasmBinary: fs.readFileSync(path.join(buildDir, 'fluffos.wasm')),
    print: (s) => console.log(s),
    printErr: (s) => console.log(s),
  });

  copyDir(Module, suiteDir, '/testsuite');
  Module.FS.chdir('/testsuite');
  Module.fluffos = { onOutput: () => {}, onDisconnect: () => {} };

  const rc = Module.ccall('fluffos_boot', 'number', ['string'], ['etc/config.test']);
  if (rc !== 0) {
    console.error('boot failed:', rc);
    process.exit(1);
  }

  try {
    const code = Module.ccall('fluffos_flag', 'number', ['string'], ['test']);
    process.exit(code);
  } catch (e) {
    // The test runner finishes by shutting the driver down: exit() inside
    // the wasm runtime surfaces here as an ExitStatus.
    if (e && e.name === 'ExitStatus') {
      process.exit(e.status === 0 ? 0 : 1);
    }
    throw e;
  }
})().catch((e) => {
  console.error('run-testsuite failed:', e);
  process.exit(1);
});
