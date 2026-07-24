// Formats a list of LPC files in place (or checks them with --check),
// with the corpus safety net: a file is only written if the formatted
// output is TOKEN-SEQUENCE-EQUIVALENT to the input (tokenize both sides,
// compare kind+text ignoring whitespace; directive tokens compare with
// trailing blanks stripped) AND the output is idempotent. A file that
// fails either check is reported and left untouched, and the run exits
// nonzero. Driven by testsuite/format.sh, which owns the file list and
// the exclusions; file paths arrive on stdin, one per line.
//
// Usage: node format-corpus.mjs [--check] < files.txt
import { readFileSync, writeFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const here = dirname(fileURLToPath(import.meta.url));
const { formatLPC } = await import(join(here, '..', 'format.mjs'));
const { tokenize } = await import(join(here, '..', 'tokenizer.mjs'));

const checkOnly = process.argv.includes('--check');
const files = readFileSync(0, 'utf8').split('\n').filter(Boolean);

function sig(src) {
  return tokenize(src).filter((t) => t.kind !== 'whitespace')
    .map((t) => t.kind + ':' + (t.kind === 'directive' ? t.text.replace(/[ \t]+$/g, '') : t.text))
    .join('\n');
}

// Literal-bearing token text must survive formatting BYTE-IDENTICAL --
// a formatter may re-flow code, never alter string/template/heredoc/
// comment/char/directive content.
const LITERAL_KINDS = new Set(['string', 'template', 'textblock', 'char', 'comment', 'directive']);
function literalText(src) {
  return tokenize(src).filter((t) => LITERAL_KINDS.has(t.kind))
    .map((t) => (t.kind === 'directive' ? t.text.replace(/[ \t]+$/g, '') : t.text))
    .join('\u0000');
}

let written = 0, unchanged = 0, wouldChange = 0, errors = 0;
for (const f of files) {
  let src;
  try {
    src = readFileSync(f, 'utf8');
  } catch (e) {
    errors++;
    console.error(`READ ERROR ${f}: ${e.message}`);
    continue;
  }
  let out;
  try {
    out = formatLPC(src);
  } catch (e) {
    errors++;
    console.error(`FORMAT ERROR ${f}: ${e.message}`);
    continue;
  }
  if (out === src) { unchanged++; continue; }
  if (sig(src) !== sig(out)) {
    errors++;
    console.error(`TOKEN MISMATCH, refusing to write: ${f}`);
    continue;
  }
  if (literalText(src) !== literalText(out)) {
    errors++;
    console.error(`LITERAL CONTENT CHANGED, refusing to write: ${f}`);
    continue;
  }
  // The idempotency pass re-formats the candidate OUTPUT; formatLPC also
  // throws if its input does not lex cleanly (unterminated string/char/
  // template/comment/text block -- a driver lexerror). Input passing that
  // same gate above does not prove the output does, so a throw here is a
  // refusal for this file, not a crash of the whole run.
  let again;
  try {
    again = formatLPC(out);
  } catch (e) {
    errors++;
    console.error(`FORMAT ERROR (output does not re-lex cleanly), refusing to write: ${f}: ${e.message}`);
    continue;
  }
  if (again !== out) {
    errors++;
    console.error(`NOT IDEMPOTENT, refusing to write: ${f}`);
    continue;
  }
  if (checkOnly) {
    wouldChange++;
    console.log(`would reformat: ${f}`);
  } else {
    writeFileSync(f, out);
    written++;
    console.log(`reformatted: ${f}`);
  }
}

console.log(JSON.stringify({ total: files.length, written, wouldChange, unchanged, errors }));
process.exit(errors > 0 || (checkOnly && wouldChange > 0) ? 1 : 0);
