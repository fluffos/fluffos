// Basic LPC formatter over the grammar-driven tokenizer: brace-depth
// reindentation, single-space operator normalization, directives at
// column 0, strings/comments/text blocks verbatim. Deterministic and
// idempotent (format(format(x)) === format(x) -- pinned by test.mjs).

import { tokenize } from './tokenizer.mjs';

const INDENT = '    ';

export function formatLPC(source) {
  const toks = tokenize(source).filter((t) => t.kind !== 'whitespace');
  const lines = [];
  let cur = [];
  let depth = 0;
  let pendingDedent = 0;

  const flush = (extraDedent = 0) => {
    if (cur.length === 0) return;
    const d = Math.max(0, depth - extraDedent);
    lines.push(INDENT.repeat(d) + renderLine(cur));
    cur = [];
  };

  // Blank-line preservation: track source line gaps.
  let lastLine = 0;
  for (let idx = 0; idx < toks.length; idx++) {
    const t = toks[idx];

    if (t.line > 0 && lastLine > 0 && t.line - lastLine > 1 && cur.length === 0
        && lines.length > 0 && lines[lines.length - 1] !== '') {
      lines.push('');
    }
    if (t.line > 0) lastLine = t.line + (t.text.match(/\n/g) || []).length;

    if (t.kind === 'directive') {
      flush();
      lines.push(t.text.replace(/[ \t]+$/g, ''));
      continue;
    }
    if (t.kind === 'comment') {
      // standalone comment gets its own line; trailing comment joins.
      if (cur.length === 0) {
        lines.push(INDENT.repeat(depth) + t.text);
      } else {
        cur.push(t);
      }
      continue;
    }

    if (t.text === '}') {
      flush();
      depth = Math.max(0, depth - 1);
      cur.push(t);
      // '}' followed by else/while(do)/';'/',' stays open on the line
      const nxt = toks[idx + 1];
      if (!nxt || !(nxt.kind === 'keyword' && (nxt.text === 'else' || nxt.text === 'while'))
          && !(nxt && nxt.kind === 'punctuation' && (nxt.text === ';' || nxt.text === ','))) {
        flush(0);
      }
      continue;
    }

    cur.push(t);

    if (t.text === '{') {
      flush(1);  // the '{' line itself sits at the outer depth
      depth++;
      continue;
    }
    if (t.text === ';' && parenDepth(cur) === 0) {
      flush();
      continue;
    }
    if (t.text === ':' && cur.length >= 2 &&
        (cur[0].text === 'case' || cur[0].text === 'default')) {
      flush(0);
      continue;
    }
  }
  flush();
  while (lines.length && lines[lines.length - 1] === '') lines.pop();
  return lines.join('\n') + '\n';
}

function parenDepth(toks) {
  let d = 0;
  for (const t of toks) {
    if (t.text === '(') d++;
    else if (t.text === ')') d--;
  }
  return d;
}

const NO_SPACE_BEFORE = new Set([';', ',', ')', ']', '}', '.', '::', '->', '?.', '.?', '++', '--', ':)']);
const NO_SPACE_AFTER = new Set(['(', '[', '{', '.', '::', '->', '?.', '.?', '!', '~', '(:']);

function renderLine(toks) {
  let out = '';
  for (let i = 0; i < toks.length; i++) {
    const t = toks[i];
    const prev = toks[i - 1];
    let sep = ' ';
    if (i === 0) sep = '';
    else if (NO_SPACE_BEFORE.has(t.text)) sep = '';
    else if (prev && NO_SPACE_AFTER.has(prev.text)) sep = '';
    else if (prev && (prev.kind === 'identifier' || prev.kind === 'type' || prev.kind === 'keyword')
             && t.text === '(') sep = prev.kind === 'keyword' && prev.text !== 'catch' ? ' ' : '';
    else if (prev && prev.kind === 'operator' && prev.text === '++') sep = '';
    else if (prev && prev.kind === 'operator' && prev.text === '--') sep = '';
    out += sep + t.text;
  }
  return out;
}
