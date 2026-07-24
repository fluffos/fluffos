// LPC tokenizer driven by lpc-grammar.json (generated from grammar.y /
// lexer.l by tools/lpc-syntax/generate_ebnf.py -- regenerate with the
// generate_ebnf CMake target; never hand-edit the JSON).
//
// Token kinds: comment, directive, string, template, textblock, char,
// number, keyword, type, modifier, efunkw, identifier, operator,
// punctuation, functional, whitespace, unknown.
//
// A spanning token that reaches end-of-input without its terminator
// (string missing its closing '"', char literal missing its closing
// "'", template literal missing its closing '`', '/*' comment missing
// its '*/', text block missing its terminator line) is emitted with
// `unterminated: true`. Driver ground truth
// (src/compiler/internal/lexer.l): every one of these is a hard
// lexerror at <<EOF>> ("End of file in string" / "End of file in a
// comment" / "End of file in template literal" / lpc_lex_char_error /
// heredoc-terminator error) -- the driver never assigns such a file a
// meaning, so consumers that rewrite source (format.mjs) must treat the
// flag as "this tokenization is nonsense past the open point" and
// refuse. One real-world stray '"' flips string/code sense for the
// whole rest of the file and the damage is invisible to any self-check
// that re-tokenizes its own output with this same tokenizer.

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
const isLexWs = (c) => c === ' ' || c === '\t' || c === '\r' || c === '\n' || c === '\v' || c === '\f';

// True when `src[i]` is a '(' immediately (modulo lexer.l's WS class:
// space/tab/CR/LF/VT/FF) followed by "::" -- i.e. a bare parent-call guard
// like `(::name(...))`, not a functional-literal open. Mirrors lexer.l's
// `"("{WS}*"::"` rule, which LPC_YYLESS(1)s back to just '(' so "::" is
// re-scanned as L_COLON_COLON.
function isParentCallOpenParen(src, i) {
  let j = i + 1;
  while (j < src.length && isLexWs(src[j])) j++;
  return src[j] === ':' && src[j + 1] === ':';
}

// Brace-depth scan for a template interpolation body: a raw '{'/'}' count
// is fooled by a '}' inside a nested string/char/comment/template (e.g.
// `${ ch == '}' }` or `${ s == "}" }`), so those spans must be skipped
// as opaque units rather than scanned character-by-character.
function skipStringSpan(src, i) {
  let j = i + 1;
  while (j < src.length && src[j] !== '"') {
    if (src[j] === '\\') j++;
    j++;
  }
  return Math.min(j + 1, src.length);
}

function skipCharSpan(src, i) {
  // Mirror lexer.l's char-literal grammar EXACTLY: after the opening
  // quote, the body is ONE unit -- either a single raw byte (any byte,
  // *including a literal quote*, per <SC_CHAR_BODY>[^\\]) or one escape
  // sequence -- and then a closing quote is required. Escapes are
  // variable-length (`'\x41'` hex, `'\101'` octal), which is why this
  // can't assume a fixed 2-char width (the old fixed width made
  // findInterpEnd swallow an interpolation's `}`), but it must NOT
  // "scan to the next quote" like skipStringSpan either: that misreads
  // the valid MudOS-ism `'''` (quote char, body is a raw `'`) as an
  // empty `''` plus a stray `'` that then opens a bogus literal running
  // to the next quote anywhere on the line -- in one real mudlib that
  // next quote sat inside a trailing `//'` comment, and the formatter
  // merged the case label, the comment, and the following statement
  // into one line, silently deleting the statement on recompile.
  let j = i + 1;
  if (j >= src.length) return j;
  if (src[j] === '\\') {
    j++; // the escape introducer; now classify per lexer.l's rules
    const e = src[j];
    if (e === undefined) return j;
    if (e === 'x') {
      // "\\x"[0-9a-fA-F]+ (or bare "\\x", an error the driver still
      // consumes as just the two chars before the close-quote check)
      j++;
      while (j < src.length && /[0-9A-Fa-f]/.test(src[j])) j++;
    } else if (e >= '0' && e <= '7') {
      // "\\"[0-7]+ octal, maximal munch
      while (j < src.length && src[j] >= '0' && src[j] <= '7') j++;
    } else if (e === '\r' && src[j + 1] === '\n') {
      j += 2; // "\\\r\n" escaped newline
    } else {
      j++; // "\\." -- simple/unknown escapes are exactly one char
    }
  } else {
    j++; // one raw body byte -- including a literal `'` or newline
  }
  // Closing quote. If it's missing the driver reports an error and
  // pushes the offending byte back for the next scan (LPC_YYLESS(0));
  // mirror that by ending the span here so the byte re-lexes normally.
  if (j < src.length && src[j] === "'") j++;
  return j;
}

function findInterpEnd(src, start) {
  let depth = 1;
  let k = start;
  while (k < src.length && depth > 0) {
    const c = src[k];
    if (c === '"') { k = skipStringSpan(src, k); continue; }
    if (c === "'") { k = skipCharSpan(src, k); continue; }
    if (c === '/' && src[k + 1] === '/') {
      const nl = src.indexOf('\n', k);
      k = nl < 0 ? src.length : nl;
      continue;
    }
    if (c === '/' && src[k + 1] === '*') {
      const e = src.indexOf('*/', k + 2);
      k = e < 0 ? src.length : e + 2;
      continue;
    }
    if (c === '`') { k = skipTemplateSpan(src, k); continue; }
    if (c === '{') { depth++; k++; continue; }
    if (c === '}') { depth--; if (depth > 0) k++; continue; }
    k++;
  }
  return k;
}

function skipTemplateSpan(src, i) {
  let j = i + 1;
  while (j < src.length) {
    if (src[j] === '\\') { j += 2; continue; }
    if (src[j] === '`') return j + 1;
    if (src[j] === '$' && src[j + 1] === '{') { j = findInterpEnd(src, j + 2) + 1; continue; }
    j++;
  }
  return j;
}

// A directive's raw '\n' terminator, scanning from `j` (inside the
// directive, past its '#'). A '/* ... */' block comment opened on a
// directive line is invisible whitespace to the directive and may close
// on a LATER physical line (docs/lpc/preprocessor/) -- the newline(s)
// inside it don't end the directive; keep scanning past the comment's
// close for the directive's REAL terminating newline instead. A '//'
// comment (which always runs to end of physical line) or a string/char
// literal's own '/*'-or-'//'-shaped content must not be misread as a
// real comment start, hence routing through skipStringSpan/skipCharSpan.
function directiveLineEnd(src, j) {
  let k = j;
  while (k < src.length && src[k] !== '\n') {
    const c = src[k];
    // String/char spans are only skipped WITHIN the physical line: the
    // driver's directive capture is strictly line-based (quotes never
    // extend a directive -- an unterminated '"' or a bare "'" in a
    // #define body is legal and inert), so a span that would run past
    // the newline means the directive ends at that newline instead.
    // Without the bound, `#define Q it'` swallowed the entire next
    // source line into the directive token, and `#define BAD "abc`
    // swallowed everything up to the next '"' anywhere in the file. (A
    // backslash-newline inside the span still continues the directive:
    // the caller's '\'-continuation check sees the '\' before this
    // returned newline, matching the driver's splice-first folding.)
    if (c === '"' || c === "'") {
      const e = c === '"' ? skipStringSpan(src, k) : skipCharSpan(src, k);
      const nl = src.indexOf('\n', k);
      if (nl >= 0 && nl < e) return nl;
      k = e;
      continue;
    }
    if (c === '/' && src[k + 1] === '*') {
      const e = src.indexOf('*/', k + 2);
      k = e < 0 ? src.length : e + 2;
      continue;
    }
    if (c === '/' && src[k + 1] === '/') {
      const nl = src.indexOf('\n', k);
      k = nl < 0 ? src.length : nl;
      continue;
    }
    k++;
  }
  return k;
}

export function tokenize(src) {
  const toks = [];
  let i = 0;
  let line = 1;
  let col = 1;
  let atLineStart = true;
  // Absolute char offset, threaded independently of `i`/`j` (the template
  // branch below does some confusing pre/post-increment bookkeeping on `i`
  // that is only used to decide where scanning resumes -- it is off by one
  // relative to actual text boundaries at times). `pos` instead mirrors the
  // line/col tracking exactly: advanced by push()'s text length, plus the
  // inner-interpolation catch-up, so it always lines up with real source
  // offsets regardless of what `i`/`j` are doing.
  let pos = 0;

  const push = (kind, text) => {
    toks.push({ kind, text, line, col, start: pos, end: pos + text.length });
    pos += text.length;
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
      const closed = j >= 0;
      j = closed ? j + 2 : src.length;
      push('comment', src.slice(i, j));
      if (!closed) toks[toks.length - 1].unterminated = true;
      i = j;
      continue;
    }

    // preprocessor directive: '#' at line start; '\'-continuations join,
    // and so does an unclosed '/* ... */' comment (see directiveLineEnd).
    if (c === '#' && atLineStart) {
      let j = i;
      for (;;) {
        let nl = directiveLineEnd(src, j);
        if (nl >= src.length) { j = src.length; break; }
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
      if (!m) toks[toks.length - 1].unterminated = true;
      i = end;
      continue;
    }

    // strings -- the loop exits either ON the closing quote (j indexes
    // it, j < length) or past end-of-input; the latter is the driver's
    // "End of file in string" lexerror, flagged for consumers.
    if (c === '"') {
      let j = i + 1;
      while (j < src.length && src[j] !== '"') {
        if (src[j] === '\\') j++;
        j++;
      }
      push('string', src.slice(i, Math.min(j + 1, src.length)));
      if (j >= src.length) toks[toks.length - 1].unterminated = true;
      i = Math.min(j + 1, src.length);
      continue;
    }

    // template literals with ${ } interpolation: emit template fragments
    // and recurse into expressions so interpolated code highlights too.
    if (c === '`') {
      let j = i + 1;
      let frag = '`';
      let closed = false;
      while (j < src.length) {
        if (src[j] === '\\') { frag += src.slice(j, j + 2); j += 2; continue; }
        if (src[j] === '`') { frag += '`'; j++; closed = true; break; }
        if (src[j] === '$' && src[j + 1] === '{') {
          push('template', frag + '${');
          i = j + 2;
          // scan interpolation with brace tracking, recursively tokenized
          const k = findInterpEnd(src, i);
          const inner = src.slice(i, k);
          const innerBase = pos;
          for (const t of tokenize(inner)) {
            toks.push({ ...t, line: 0, col: 0, start: t.start + innerBase, end: t.end + innerBase });
          }
          // account positions for inner text
          for (const ch of inner) {
            if (ch === '\n') { line++; col = 1; } else { col++; }
          }
          pos += inner.length;
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
      if (!closed) toks[toks.length - 1].unterminated = true;
      i = j;
      continue;
    }

    // char literal -- one body byte or escape, then the closing quote,
    // exactly as lexer.l scans it (see skipCharSpan for the full rule;
    // notably `'''` is a VALID quote-char literal, not an empty `''`).
    if (c === "'") {
      const j = skipCharSpan(src, i);
      const span = src.slice(i, j);
      push('char', span);
      // A char literal without its closing quote is a driver lexerror
      // whether it hits EOF ("End of file in char") or not (push-back
      // recovery, compile still fails): flag it so formatLPC refuses the
      // file (skipCharSpan ends the span without the close in that case).
      if (!(span.length >= 2 && span.endsWith("'"))) {
        toks[toks.length - 1].unterminated = true;
      }
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
    //
    // "(::" (optionally with whitespace between the '(' and the "::") is
    // NOT a functional-literal open -- it's an ordinary '(' followed by
    // the "::" scope-resolution operator, as in a bare parent-call guard
    // `if (::name(...))`. lexer.l's own "("{WS}*"::" rule exists for
    // exactly this: it returns just '(' and pushes the rest back so "::"
    // scans as its own token next (see the "(::" longest-match guard
    // comment there). Without this guard, greedily matching '(' + ':' as
    // "(:" leaves a lone ':' behind, corrupting the token stream (`::` ->
    // `: :`) and everything the formatter builds on top of it.
    if (c === '(' && src[i + 1] === ':') {
      if (isParentCallOpenParen(src, i)) {
        // Bare '(' -- do not let the operator table below match "(:" as a
        // functional-literal open either; emit just the paren and let "::"
        // (and any whitespace between them) scan on the next iterations.
        push('punctuation', '(');
        i += 1;
        continue;
      }
      push('functional', '(:'); i += 2; continue;
    }
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
