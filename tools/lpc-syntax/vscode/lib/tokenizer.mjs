// GENERATED COPY -- edit tools/lpc-syntax/tokenizer.mjs and re-run the
// generate_ebnf CMake target; a packaged VS Code extension
// cannot reach outside its own folder.
// LPC tokenizer driven by lpc-grammar.json (generated from grammar.y /
// lexer.l by tools/lpc-syntax/generate_ebnf.py -- regenerate with the
// generate_ebnf CMake target; never hand-edit the JSON).
//
// Token kinds: comment, directive, string, template, textblock, char,
// number, keyword, type, modifier, efunkw, identifier, operator,
// punctuation, functional, whitespace, unknown.

import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const here = dirname(fileURLToPath(import.meta.url));
export const grammar = JSON.parse(readFileSync(join(here, 'lpc-grammar.json'), 'utf8'));

const KEYWORDS = new Set(grammar.keywords);
const TYPES = new Set(grammar.typeKeywords);
const MODIFIERS = new Set(grammar.modifierKeywords);
// Longest-match order comes pre-sorted from the generator.
const OPERATORS = grammar.operators;
const PUNCT = new Set(grammar.punctuation);

const isIdentStart = (c) => /[A-Za-z_]/.test(c);
const isIdentChar = (c) => /[A-Za-z0-9_]/.test(c);
const isDigit = (c) => /[0-9]/.test(c);

export function tokenize(src) {
  const toks = [];
  let i = 0;
  let line = 1;
  let col = 1;
  let atLineStart = true;

  const push = (kind, text) => {
    toks.push({ kind, text, line, col });
    for (const ch of text) {
      if (ch === '\n') { line++; col = 1; } else { col++; }
    }
    if (kind !== 'whitespace' && kind !== 'comment') atLineStart = false;
  };

  const readWhile = (pred) => {
    let j = i;
    while (j < src.length && pred(src[j])) j++;
    return src.slice(i, j);
  };

  while (i < src.length) {
    const c = src[i];

    // whitespace (newline re-arms directive detection)
    if (c === ' ' || c === '\t' || c === '\r' || c === '\n') {
      const t = readWhile((ch) => ch === ' ' || ch === '\t' || ch === '\r' || ch === '\n');
      if (t.includes('\n')) atLineStart = true;
      push('whitespace', t);
      i += t.length;
      continue;
    }

    // comments
    if (c === '/' && src[i + 1] === '/') {
      let j = src.indexOf('\n', i);
      if (j < 0) j = src.length;
      push('comment', src.slice(i, j));
      i = j;
      continue;
    }
    if (c === '/' && src[i + 1] === '*') {
      let j = src.indexOf('*/', i + 2);
      j = j < 0 ? src.length : j + 2;
      push('comment', src.slice(i, j));
      i = j;
      continue;
    }

    // preprocessor directive: '#' at line start; '\'-continuations join
    if (c === '#' && atLineStart) {
      let j = i;
      for (;;) {
        let nl = src.indexOf('\n', j);
        if (nl < 0) { j = src.length; break; }
        let k = nl - 1;
        while (k > j && src[k] === '\r') k--;
        if (src[k] === '\\') { j = nl + 1; continue; }
        j = nl;
        break;
      }
      push('directive', src.slice(i, j));
      i = j;
      continue;
    }

    // text blocks: @TERM / @@TERM ... TERM at line start
    if (c === '@' && isIdentStart(src[i + 1] === '@' ? src[i + 2] ?? '' : src[i + 1] ?? '')) {
      const arr = src[i + 1] === '@';
      let j = i + (arr ? 2 : 1);
      let term = '';
      while (j < src.length && isIdentChar(src[j])) { term += src[j]; j++; }
      const endRe = new RegExp(`^${term}(?![A-Za-z0-9_])`, 'm');
      const rest = src.slice(j);
      const m = endRe.exec(rest);
      let end;
      if (m) end = j + m.index + term.length;
      else end = src.length;
      push('textblock', src.slice(i, end));
      i = end;
      continue;
    }

    // strings
    if (c === '"') {
      let j = i + 1;
      while (j < src.length && src[j] !== '"') {
        if (src[j] === '\\') j++;
        j++;
      }
      push('string', src.slice(i, Math.min(j + 1, src.length)));
      i = Math.min(j + 1, src.length);
      continue;
    }

    // template literals with ${ } interpolation: emit template fragments
    // and recurse into expressions so interpolated code highlights too.
    if (c === '`') {
      let j = i + 1;
      let frag = '`';
      while (j < src.length) {
        if (src[j] === '\\') { frag += src.slice(j, j + 2); j += 2; continue; }
        if (src[j] === '`') { frag += '`'; j++; break; }
        if (src[j] === '$' && src[j + 1] === '{') {
          push('template', frag + '${');
          i = j + 2;
          // scan interpolation with brace tracking, recursively tokenized
          let depth = 1;
          let k = i;
          while (k < src.length && depth > 0) {
            if (src[k] === '{') depth++;
            else if (src[k] === '}') depth--;
            if (depth > 0) k++;
          }
          const inner = src.slice(i, k);
          for (const t of tokenize(inner)) {
            toks.push({ ...t, line: 0, col: 0 });
          }
          // account positions for inner text
          for (const ch of inner) {
            if (ch === '\n') { line++; col = 1; } else { col++; }
          }
          i = k;
          frag = '}';
          j = i + 1;
          if (src[i] === '}') { j = i + 1; i = j; j = i; }
          j = i;
          continue;
        }
        frag += src[j];
        j++;
      }
      push('template', frag);
      i = j;
      continue;
    }

    // char literal
    if (c === "'") {
      let j = i + 1;
      if (src[j] === '\\') j++;
      j++;
      if (src[j] === "'") j++;
      push('char', src.slice(i, j));
      i = j;
      continue;
    }

    // numbers: 0x/0b, underscores, reals
    if (isDigit(c)) {
      let j = i;
      if (c === '0' && (src[j + 1] === 'x' || src[j + 1] === 'X')) {
        j += 2;
        while (j < src.length && /[0-9A-Fa-f_]/.test(src[j])) j++;
      } else if (c === '0' && (src[j + 1] === 'b' || src[j + 1] === 'B')) {
        j += 2;
        while (j < src.length && /[01_]/.test(src[j])) j++;
      } else {
        while (j < src.length && /[0-9_]/.test(src[j])) j++;
        // Float: optional fraction (or trailing dot -- never consuming
        // the ".." range operator), then an optional exponent; a bare
        // exponent ("1e3") is a float too. "1e" with no digits is
        // NUMBER(1) IDENT(e), so the exponent needs a lookahead digit.
        if (src[j] === '.' && src[j + 1] !== '.') {
          j++;
          while (j < src.length && /[0-9_]/.test(src[j])) j++;
        }
        if ((src[j] === 'e' || src[j] === 'E') &&
            (isDigit(src[j + 1] ?? '') ||
             ((src[j + 1] === '+' || src[j + 1] === '-') && isDigit(src[j + 2] ?? '')))) {
          j++;
          if (src[j] === '+' || src[j] === '-') j++;
          while (j < src.length && /[0-9_]/.test(src[j])) j++;
        }
      }
      push('number', src.slice(i, j));
      i = j;
      continue;
    }

    // identifiers / keywords ($N parameters too)
    if (isIdentStart(c) || (c === '$' && isDigit(src[i + 1] ?? ''))) {
      let j = i + (c === '$' ? 1 : 0);
      while (j < src.length && isIdentChar(src[j])) j++;
      const word = src.slice(i, j);
      let kind = 'identifier';
      if (KEYWORDS.has(word)) kind = word === 'efun' ? 'efunkw' : 'keyword';
      else if (TYPES.has(word)) kind = 'type';
      else if (MODIFIERS.has(word)) kind = 'modifier';
      push(kind, word);
      i = j;
      continue;
    }

    // functional open/close before operators ("(:", ":)")
    if (c === '(' && src[i + 1] === ':') { push('functional', '(:'); i += 2; continue; }
    if (c === ':' && src[i + 1] === ')') { push('functional', ':)'); i += 2; continue; }

    // operators, longest-match from the grammar contract
    let matched = false;
    for (const op of OPERATORS) {
      if (src.startsWith(op, i)) {
        push('operator', op);
        i += op.length;
        matched = true;
        break;
      }
    }
    if (matched) continue;

    if (PUNCT.has(c)) {
      push('punctuation', c);
      i++;
      continue;
    }

    push('unknown', c);
    i++;
  }
  return toks;
}
