// GENERATED COPY -- edit tools/lpc-syntax/lint.mjs and re-run the
// generate_ebnf CMake target; a packaged VS Code extension
// cannot reach outside its own folder.
// Structural LPC lint over the grammar-driven tokenizer: fast, pure,
// dependency-free checks a text editor can run on every keystroke.
// Deeper errors come from the real compiler (the VS Code extension can
// run `lpcc` on save); this catches what a tokenizer can prove:
//   - illegal characters
//   - unterminated strings / templates / block comments / text blocks
//   - unbalanced () [] {} (string/comment content excluded by tokenizing)
//   - mismatched #if/#ifdef/#ifndef ... #elif/#else/#endif
//
// lintLPC(source) -> [{ line, col, endLine, endCol, severity, message }]
// (1-based lines/cols, end exclusive; severity 'error' | 'warning')

import { tokenize } from './tokenizer.mjs';

const OPEN = { '(': ')', '[': ']', '{': '}' };
const CLOSE = { ')': '(', ']': '[', '}': '{' };

function tokenEnd(t) {
  let line = t.line;
  let col = t.col;
  for (const ch of t.text) {
    if (ch === '\n') { line++; col = 1; } else { col++; }
  }
  return { line, col };
}

function diag(t, severity, message, len = 0) {
  const end = len > 0 ? { line: t.line, col: t.col + len } : tokenEnd(t);
  return { line: t.line, col: t.col, endLine: end.line, endCol: end.col, severity, message };
}

export function lintLPC(source) {
  const out = [];
  const toks = tokenize(source);

  const brackets = [];
  const conds = [];

  for (const t of toks) {
    // Interpolated template tokens are re-tokenized without positions
    // (line 0); structural checks would misreport them, skip.
    if (t.line === 0) continue;

    switch (t.kind) {
      case 'unknown':
        out.push(diag(t, 'error', `Illegal character '${t.text}'`));
        break;

      case 'string': {
        const s = t.text;
        const closed = s.length >= 2 && s.endsWith('"') &&
                       (!s.endsWith('\\"') || s.endsWith('\\\\"'));
        if (!closed) out.push(diag(t, 'error', 'Unterminated string literal', 1));
        break;
      }

      case 'template': {
        // Only the FINAL fragment of a template carries the closing
        // backtick; middle fragments end with "${". A fragment ending in
        // neither (and not opening an interpolation) hit end-of-file.
        const s = t.text;
        if (!s.endsWith('`') && !s.endsWith('${')) {
          out.push(diag(t, 'error', 'Unterminated template literal', 1));
        } else if (s.length === 1 && s === '`') {
          out.push(diag(t, 'error', 'Unterminated template literal', 1));
        }
        break;
      }

      case 'comment':
        if (t.text.startsWith('/*') && !t.text.endsWith('*/')) {
          out.push(diag(t, 'error', 'Unterminated block comment', 2));
        }
        break;

      case 'textblock': {
        // "@TERM\n...\nTERM" -- the tokenizer consumed to EOF if the
        // terminator never appeared at line start.
        const m = /^@@?([A-Za-z_][A-Za-z0-9_]*)/.exec(t.text);
        if (m) {
          const endRe = new RegExp(`(?:^|\\n)${m[1]}(?![A-Za-z0-9_])[^\\n]*$`);
          if (!endRe.test(t.text)) {
            out.push(diag(t, 'error',
              `Text block not terminated ('${m[1]}' at start of a line)`, m[0].length));
          }
        }
        break;
      }

      case 'punctuation': {
        const c = t.text;
        if (OPEN[c] !== undefined) {
          brackets.push(t);
        } else if (CLOSE[c] !== undefined) {
          const top = brackets[brackets.length - 1];
          if (top === undefined) {
            out.push(diag(t, 'error', `Unmatched '${c}'`));
          } else if (OPEN[top.text] !== c) {
            out.push(diag(t, 'error',
              `Mismatched '${c}' -- expected '${OPEN[top.text]}' (opened at line ${top.line})`));
            brackets.pop();
          } else {
            brackets.pop();
          }
        }
        break;
      }

      case 'directive': {
        const m = /^#\s*([A-Za-z_]+)/.exec(t.text);
        const name = m ? m[1] : '';
        const nameLen = m ? m[0].length : 1;
        if (name === 'if' || name === 'ifdef' || name === 'ifndef') {
          conds.push(t);
        } else if (name === 'elif' || name === 'else') {
          if (conds.length === 0) {
            out.push(diag(t, 'error', `Unexpected #${name}`, nameLen));
          }
        } else if (name === 'endif') {
          if (conds.length === 0) {
            out.push(diag(t, 'error', 'Unexpected #endif', nameLen));
          } else {
            conds.pop();
          }
        }
        break;
      }

      default:
        break;
    }
  }

  for (const t of brackets) {
    out.push(diag(t, 'error', `Unclosed '${t.text}'`));
  }
  for (const t of conds) {
    const m = /^#\s*[A-Za-z_]+/.exec(t.text);
    out.push(diag(t, 'error', 'Missing #endif', m ? m[0].length : 1));
  }
  return out;
}
