// LPC (FluffOS) VS Code extension.
//
// Diagnostics come from two sources:
//  1. The built-in structural lint (lib/lint.mjs over lib/tokenizer.mjs,
//     both generated copies of the grammar-driven tooling one level up)
//     -- runs on open/edit, catches what a tokenizer can prove.
//  2. Optionally the REAL compiler: when lpc.lpcc.path and
//     lpc.lpcc.configFile are configured, the file is compiled with
//     lpcc on save and its clang-style diagnostics are parsed
//     (file:line:col: severity: message).
//
// Highlighting is declarative (syntaxes/lpc.tmLanguage.json, generated
// from the compiler's grammar contract) -- no code here.
//
// Formatting (Format Document / format-on-save) runs lib/format.mjs, a
// generated copy of the grammar-driven formatter one level up. A bad
// format never blocks a save: errors and no-op results yield no edits.

'use strict';

const vscode = require('vscode');
const path = require('path');
const cp = require('child_process');
const { pathToFileURL } = require('url');

let lintPromise = null;
let formatPromise = null;

function loadLint(ctx) {
  if (lintPromise === null) {
    const url = pathToFileURL(path.join(ctx.extensionPath, 'lib', 'lint.mjs')).href;
    lintPromise = import(url).then((m) => m.lintLPC);
  }
  return lintPromise;
}

function loadFormat(ctx) {
  if (formatPromise === null) {
    const url = pathToFileURL(path.join(ctx.extensionPath, 'lib', 'format.mjs')).href;
    formatPromise = import(url).then((m) => m.formatLPC);
  }
  return formatPromise;
}

function toRange(d) {
  // lint.mjs reports 1-based inclusive start / exclusive end.
  const endCol = d.endCol > d.col || d.endLine > d.line ? d.endCol : d.col + 1;
  return new vscode.Range(d.line - 1, d.col - 1, (d.endLine || d.line) - 1, endCol - 1);
}

async function runLint(doc, coll, ctx) {
  if (doc.languageId !== 'lpc') return;
  const cfg = vscode.workspace.getConfiguration('lpc', doc.uri);
  if (!cfg.get('lint.enabled', true)) {
    coll.set(doc.uri, []);
    return;
  }
  const lintLPC = await loadLint(ctx);
  const diags = lintLPC(doc.getText()).map((d) => {
    const diag = new vscode.Diagnostic(
      toRange(d), d.message,
      d.severity === 'warning' ? vscode.DiagnosticSeverity.Warning
                               : vscode.DiagnosticSeverity.Error);
    diag.source = 'lpc-lint';
    return diag;
  });
  coll.set(doc.uri, diags);
}

// --- lpcc integration -------------------------------------------------------

// One diagnostic line: /path/file.lpc:12:5: error: message
const LPCC_DIAG_RE = /^\/?(.+?):(\d+):(\d+): (error|warning): (.*)$/;

function parseLpccOutput(text, mudlibRoot) {
  // uri path (mudlib-resolved) -> vscode.Diagnostic[]
  const byFile = new Map();
  for (const line of text.split(/\r?\n/)) {
    const m = LPCC_DIAG_RE.exec(line);
    if (!m) continue;
    const [, file, lineNo, colNo, sev, msg] = m;
    const abs = path.join(mudlibRoot, file);
    const l = Math.max(0, parseInt(lineNo, 10) - 1);
    const c = Math.max(0, parseInt(colNo, 10) - 1);
    const diag = new vscode.Diagnostic(
      new vscode.Range(l, c, l, c + 1), msg,
      sev === 'warning' ? vscode.DiagnosticSeverity.Warning
                        : vscode.DiagnosticSeverity.Error);
    diag.source = 'lpcc';
    if (!byFile.has(abs)) byFile.set(abs, []);
    byFile.get(abs).push(diag);
  }
  return byFile;
}

function lpccSettings(doc) {
  const cfg = vscode.workspace.getConfiguration('lpc', doc.uri);
  const lpcc = cfg.get('lpcc.path', '');
  const configFile = cfg.get('lpcc.configFile', '');
  if (!lpcc || !configFile) return null;
  let mudlibRoot = cfg.get('mudlibRoot', '');
  if (!mudlibRoot) {
    const folder = vscode.workspace.getWorkspaceFolder(doc.uri);
    if (!folder) return null;
    mudlibRoot = folder.uri.fsPath;
  }
  return { lpcc, configFile, mudlibRoot };
}

function runLpcc(doc, coll) {
  const s = lpccSettings(doc);
  if (s === null) return;
  const rel = path.relative(s.mudlibRoot, doc.fileName).split(path.sep).join('/');
  if (rel.startsWith('..')) return; // outside the mudlib
  cp.execFile(
    s.lpcc, [s.configFile, rel],
    { cwd: s.mudlibRoot, timeout: 30000, maxBuffer: 4 * 1024 * 1024 },
    (_err, stdout, stderr) => {
      // lpcc exits nonzero on compile errors -- the diagnostics ARE the
      // result, so the error object itself is ignored.
      const byFile = parseLpccOutput(String(stderr) + '\n' + String(stdout), s.mudlibRoot);
      coll.clear();
      for (const [file, diags] of byFile) {
        coll.set(vscode.Uri.file(file), diags);
      }
    });
}

// --- activation --------------------------------------------------------------

function activate(ctx) {
  const lintColl = vscode.languages.createDiagnosticCollection('lpc-lint');
  const lpccColl = vscode.languages.createDiagnosticCollection('lpcc');
  ctx.subscriptions.push(lintColl, lpccColl);

  const timers = new Map();
  const debounced = (doc) => {
    const key = doc.uri.toString();
    clearTimeout(timers.get(key));
    timers.set(key, setTimeout(() => runLint(doc, lintColl, ctx), 300));
  };

  ctx.subscriptions.push(
    vscode.workspace.onDidOpenTextDocument((doc) => runLint(doc, lintColl, ctx)),
    vscode.workspace.onDidChangeTextDocument((e) => debounced(e.document)),
    vscode.workspace.onDidSaveTextDocument((doc) => {
      runLint(doc, lintColl, ctx);
      if (doc.languageId === 'lpc') runLpcc(doc, lpccColl);
    }),
    vscode.workspace.onDidCloseTextDocument((doc) => {
      lintColl.delete(doc.uri);
    }));

  for (const doc of vscode.workspace.textDocuments) {
    runLint(doc, lintColl, ctx);
  }

  ctx.subscriptions.push(
    vscode.languages.registerDocumentFormattingEditProvider('lpc', {
      provideDocumentFormattingEdits: async (doc) => {
        const cfg = vscode.workspace.getConfiguration('lpc', doc.uri);
        if (!cfg.get('format.enabled', true)) return [];
        try {
          const formatLPC = await loadFormat(ctx);
          const printWidth = cfg.get('format.printWidth', 100);
          const indentSize = cfg.get('format.indentSize', 2);
          const formatted = formatLPC(doc.getText(), { printWidth, indentSize });
          if (!formatted || formatted === doc.getText()) return [];
          const fullRange = new vscode.Range(
            doc.positionAt(0), doc.positionAt(doc.getText().length));
          return [vscode.TextEdit.replace(fullRange, formatted)];
        } catch (_err) {
          return [];
        }
      },
    }));
}

function deactivate() {}

module.exports = { activate, deactivate };
