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
  // Running paren nesting across the whole pass (not just the current
  // `cur` line buffer) -- a forced mid-statement flush (e.g. after a
  // heredoc terminator, or a multiline-literal element break) can leave
  // `cur` holding an unmatched ')' with no '(' of its own, which would
  // make a per-cur-buffer depth count go negative and never reach 0.
  let parenLevel = 0;
  // '({' ... '})' is an array literal and '([' ... '])' a mapping literal,
  // not blocks or indexing -- one combined LIFO stack (nesting order can
  // interleave '{' and '[', e.g. `([ "a": ({ ... }) ])`) tags each open
  // bracket by whether it's immediately preceded by '(': array/mapping if
  // so, block/index (unchanged behavior) otherwise. A literal starts
  // single-line (no depth change, no forced break, same as before); the
  // first time an element is seen starting on a later source line than
  // the previous token, it flips to 'multiline' -- from then on it gets
  // one real indent level (via `depth`) like a block, so nested blocks
  // inside it (e.g. a closure literal) indent correctly on top of it.
  const litStack = [];

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
    if (t.kind === 'textblock') {
      // A heredoc's terminator word ends the token mid-line (e.g. at
      // "ENDHELP" in "ENDHELP);" -- the driver rescans anything after it
      // on that line as ordinary code, see parseHeredoc() in
      // lexer_utils.cc). Force a break so that trailing code renders on
      // its own line, matching the documented @/@@ style, instead of
      // gluing it onto the terminator (e.g. "ENDHELP, 1);").
      cur.push(t);
      flush();
      continue;
    }

    if (t.text === '}' || t.text === ']') {
      const top = litStack.pop();
      if (top && (top.kind === 'array' || top.kind === 'mapping')) {
        if (top.multiline) {
          if (cur.length > 0) flush();
          depth = Math.max(0, depth - 1);
        }
        cur.push(t);
        continue;
      }
      if (top && top.kind === 'index') {
        cur.push(t);
        continue;
      }
      // block close ('}' -- '[' never opens a block)
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

    // Inside a literal that's already gone multiline (or is starting to),
    // a token beginning on a later source line than the previous one
    // preserves that break -- one element per line, matching how the
    // source laid out a large mapping/array table instead of smashing it
    // onto a single line.
    if (litStack.length) {
      const top = litStack[litStack.length - 1];
      if ((top.kind === 'array' || top.kind === 'mapping') && cur.length > 0 &&
          t.line > 0 && cur[cur.length - 1].line > 0 && t.line > cur[cur.length - 1].line) {
        flush();
        if (!top.multiline) { top.multiline = true; depth++; }
      }
    }

    cur.push(t);
    if (t.text === '(') parenLevel++;
    else if (t.text === ')') parenLevel = Math.max(0, parenLevel - 1);

    if (t.text === '{' || t.text === '[') {
      const prev = prevNonComment(toks, idx);
      const isLiteral = !!prev && prev.text === '(';
      if (t.text === '{') {
        litStack.push({ kind: isLiteral ? 'array' : 'block', multiline: false });
        if (isLiteral) continue;
        flush();  // the '{' line itself sits at the CURRENT depth --
                  // depth only increments for what comes after it
        depth++;
        continue;
      }
      litStack.push({ kind: isLiteral ? 'mapping' : 'index', multiline: false });
      continue;
    }
    if (t.text === ';' && parenLevel === 0) {
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

function prevNonComment(toks, idx) {
  for (let k = idx - 1; k >= 0; k--) {
    if (toks[k].kind !== 'comment') return toks[k];
  }
  return null;
}

// '[' opens indexing (`a[0]`) or, right after '(', a mapping literal
// (`([...])`) -- neither ever wants a space before it. '..' is the range
// operator (`a[0..2]`) and stays tight on both sides, unlike '...'
// varargs/spread which keeps normal spacing (`f(int a, ...)`).
const NO_SPACE_BEFORE = new Set([';', ',', ')', ']', '}', '.', '::', '->', '?.', '.?', '++', '--', ':)', '[', '..']);
const NO_SPACE_AFTER = new Set(['(', '[', '{', '.', '::', '->', '?.', '.?', '!', '~', '(:', '..']);

function renderLine(toks) {
  let out = '';
  for (let i = 0; i < toks.length; i++) {
    const t = toks[i];
    const prev = toks[i - 1];
    let sep = ' ';
    if (i === 0) sep = '';
    else if (NO_SPACE_BEFORE.has(t.text)) sep = '';
    // `case X:` / `default:` -- no space before the label colon (near-
    // universal house style; unlike the ternary and mapping-literal ':',
    // which do get a leading space).
    else if (t.text === ':' && (toks[0].text === 'case' || toks[0].text === 'default')) sep = '';
    // `..<` -- the exclusive range-bound marker (`a[..<j]`) -- stays tight
    // on its far side too; bare '<' (less-than) elsewhere keeps its space.
    else if (prev && prev.text === '<' && toks[i - 2] && toks[i - 2].text === '..') sep = '';
    else if (prev && NO_SPACE_AFTER.has(prev.text)) sep = '';
    else if (prev && (prev.kind === 'identifier' || prev.kind === 'type' || prev.kind === 'keyword')
             && t.text === '(') sep = prev.kind === 'keyword' && prev.text !== 'catch' ? ' ' : '';
    else if (prev && prev.kind === 'operator' && prev.text === '++') sep = '';
    else if (prev && prev.kind === 'operator' && prev.text === '--') sep = '';
    out += sep + t.text;
  }
  return out;
}
