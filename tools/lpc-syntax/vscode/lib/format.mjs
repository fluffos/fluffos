// GENERATED COPY -- edit tools/lpc-syntax/format.mjs and re-run the
// generate_ebnf CMake target; a packaged VS Code extension
// cannot reach outside its own folder.
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
  // '({' ... '})' is an array/mapping literal, not a block -- its braces
  // must not affect indentation depth or force line breaks. Track each
  // '{' as literal-vs-block by whether it's immediately preceded by '(',
  // and pop the matching kind on '}' (LIFO, so nesting is exact).
  const braceStack = [];

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
      // A '//' comment runs to end of physical line, so nothing may
      // follow it on the same rendered line -- force a flush, or a
      // second format pass would swallow the next tokens into it.
      if (cur.length === 0) {
        lines.push(INDENT.repeat(depth) + t.text);
      } else {
        cur.push(t);
        if (t.text.startsWith('//')) flush();
      }
      continue;
    }

    if (t.text === '}') {
      const wasArray = braceStack.pop() === 'array';
      if (wasArray) {
        cur.push(t);
        continue;
      }
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
      const prev = prevNonComment(toks, idx);
      const isArray = !!prev && prev.text === '(';
      braceStack.push(isArray ? 'array' : 'block');
      if (isArray) continue;
      flush();  // the '{' line itself sits at the CURRENT depth --
                // depth only increments for what comes after it
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
  // A line entry can itself embed raw newlines (an unterminated string/
  // comment/text block token swallows to EOF, trailing newline included)
  // -- don't add a second one, or each re-format grows another blank line.
  const joined = lines.join('\n');
  return joined.endsWith('\n') ? joined : joined + '\n';
}

function parenDepth(toks) {
  let d = 0;
  for (const t of toks) {
    if (t.text === '(') d++;
    else if (t.text === ')') d--;
  }
  return d;
}

function prevNonComment(toks, idx) {
  for (let k = idx - 1; k >= 0; k--) {
    if (toks[k].kind !== 'comment') return toks[k];
  }
  return null;
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
