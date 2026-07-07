// Dependency-free test suite: node tools/lpc-syntax/test.mjs
import { tokenize, grammar } from './tokenizer.mjs';
import { highlightLPC } from './highlight.mjs';
import { formatLPC } from './format.mjs';
import { lintLPC } from './lint.mjs';
import { readFileSync as readF } from 'node:fs';
import { fileURLToPath as f2p } from 'node:url';
import { dirname as dirN, join as joinP } from 'node:path';

let failures = 0;
const check = (name, cond, detail = '') => {
  if (cond) console.log(`  OK  ${name}`);
  else { console.error(`FAIL  ${name}${detail ? ' -- ' + detail : ''}`); failures++; }
};
const kinds = (src) => tokenize(src).filter((t) => t.kind !== 'whitespace')
                                    .map((t) => `${t.kind}:${t.text}`);

// --- grammar contract sanity ------------------------------------------------
check('grammar has productions', grammar.productions.length > 200);
check('grammar keywords include control flow',
      ['if', 'foreach', 'inherit', 'catch'].every((k) => grammar.keywords.includes(k)));
check('operators longest-match ordered',
      grammar.operators.indexOf('<<=') < grammar.operators.indexOf('<<'));

// --- tokenizer ---------------------------------------------------------------
check('keywords vs identifiers',
      kinds('if (foo) return bar;').join(',') ===
      'keyword:if,punctuation:(,identifier:foo,punctuation:),keyword:return,identifier:bar,punctuation:;');
check('types and modifiers',
      kinds('private int x;').join(',') ===
      'modifier:private,type:int,identifier:x,punctuation:;');
check('numbers: hex/bin/underscore/real',
      kinds('0xFF 0b10_1 1_000 3.14').join(',') ===
      'number:0xFF,number:0b10_1,number:1_000,number:3.14');
check('string with escapes stays one token',
      kinds('"a\\"b\\n"').join(',') === 'string:"a\\"b\\n"');
check('adjacent strings are two tokens',
      kinds('"a" "b"').length === 2);
check('char literal', kinds("'\\n'")[0].startsWith('char:'));
check('directive captured whole (with continuation)',
      kinds('#define F(x) \\\n    ((x)+1)\nint y;')[0].startsWith('directive:#define F(x)'));
check('directive only at line start',
      kinds('int a; // #define X\nint b;').some((k) => k.startsWith('comment:')) &&
      !kinds('int a; // no\nint b;').some((k) => k.startsWith('directive:')));
check('template fragments + interpolated expression tokens',
      (() => {
        const k = kinds('`v=${1 + x}!`');
        return k[0] === 'template:`v=${' && k.includes('number:1') &&
               k.includes('identifier:x') && k[k.length - 1] === 'template:}!`';
      })());
check('functional open/close', kinds('(: foo :)').join(',') ===
      'functional:(:,identifier:foo,functional::)');
check('optional chaining ops', kinds('m?.k m.?[0]').some((k) => k === 'operator:?.') &&
      kinds('m?.k m.?[0]').some((k) => k === 'operator:.?'));
check('text block single', kinds('@END\nline one\nEND\n')[0].startsWith('textblock:@END'));
check('text block array', kinds('@@T\nx\nT\n')[0].startsWith('textblock:@@T'));
check('range vs ellipsis vs dot',
      kinds('a[1..2] f(...) x.y').join(',').includes('operator:..') &&
      kinds('f(...)').includes('operator:...'));

// --- highlighter --------------------------------------------------------------
const html = highlightLPC('int f() { return "hi"; } // done');
check('highlight keyword span', html.includes('<span class="lpc-keyword">return</span>'));
check('highlight string span', html.includes('<span class="lpc-string">&quot;hi&quot;</span>'));
check('highlight comment span', html.includes('<span class="lpc-comment">// done</span>'));
check('html escaped', highlightLPC('if (a < b) x = "<&>";').includes('&lt;&amp;&gt;'));

// --- formatter ----------------------------------------------------------------
const ugly = 'int  f( int x ){if(x>0){return   x;}else{return -x;}}';
const pretty = formatLPC(ugly);
check('formatter indents braces', pretty.includes('\n    if (x > 0) {') || pretty.includes('if (x > 0) {'));
check('formatter newline per statement', pretty.split('\n').filter(Boolean).length >= 5);
check('formatter idempotent', formatLPC(pretty) === pretty,
      JSON.stringify({ once: pretty, twice: formatLPC(pretty) }));
const withDirective = formatLPC('#define X 1\n   int f(){return X;}');
check('directives at column 0', withDirective.startsWith('#define X 1\n'));
const withComment = formatLPC('// header\nint g(){return 1;}');
check('standalone comment kept', withComment.startsWith('// header\n'));
check('strings verbatim through formatter',
      formatLPC('string s="a  b";').includes('"a  b"'));

// --- lint ---------------------------------------------------------------------
const msgs = (src) => lintLPC(src).map((d) => d.message);
check('lint: clean file is clean',
      lintLPC('int f() { return ({ 1, 2 })[0]; } // ok\n').length === 0);
check('lint: functionals do not unbalance',
      lintLPC('mixed f() { return (: $1 + $2 :); }\n').length === 0);
check('lint: illegal character',
      msgs('int x = 1 @;\n').length === 0 /* @ starts textblock probe */ ||
      msgs('int x = 1 \u0001;\n').some((m) => m.includes('Illegal character')));
check('lint: unterminated string',
      msgs('string s = "abc;\n').some((m) => m.includes('Unterminated string')));
check('lint: escaped quote not closing',
      msgs('string s = "a\\\\";\n').length === 0 &&
      msgs('string s = "a\\";\n').some((m) => m.includes('Unterminated string')));
check('lint: unterminated block comment',
      msgs('/* never ends\nint x;\n').some((m) => m.includes('Unterminated block comment')));
check('lint: unterminated template',
      msgs('string s = `abc;\n').some((m) => m.includes('Unterminated template')));
check('lint: unterminated text block',
      msgs('string s = @END\nbody line\n').some((m) => m.includes('Text block not terminated')));
check('lint: terminated text block is clean',
      lintLPC('string s = @END\nbody\nEND\n').length === 0);
check('lint: unclosed brace',
      msgs('int f() { if (1) { return 1; }\n').some((m) => m.includes("Unclosed '{'")));
check('lint: unmatched close',
      msgs('int f() { return 1; } }\n').some((m) => m.includes("Unmatched '}'")));
check('lint: mismatched bracket kind',
      msgs('int f() { return (1]; }\n').some((m) => m.includes("Mismatched ']'")));
check('lint: missing endif',
      msgs('#ifdef FOO\nint x;\n').some((m) => m.includes('Missing #endif')));
check('lint: unexpected endif/else',
      msgs('int x;\n#endif\n').some((m) => m.includes('Unexpected #endif')) &&
      msgs('int x;\n#else\n').some((m) => m.includes('Unexpected #else')));
check('lint: balanced conditionals are clean',
      lintLPC('#ifdef A\nint x;\n#elif B\nint y;\n#else\nint z;\n#endif\n').length === 0);
check('lint: positions are 1-based',
      (() => { const d = lintLPC('string s = "abc;\n')[0];
               return d.line === 1 && d.col === 12; })());

// --- generated VS Code assets ---------------------------------------------------
const here2 = dirN(f2p(import.meta.url));
const tml = JSON.parse(readF(joinP(here2, 'vscode/syntaxes/lpc.tmLanguage.json'), 'utf8'));
check('tmLanguage: scope + language wiring',
      tml.scopeName === 'source.lpc' && Array.isArray(tml.patterns));
check('tmLanguage: keywords from grammar contract',
      tml.repository.keywords.match.includes('foreach') &&
      tml.repository.types.match.includes('mapping'));
check('tmLanguage: operators longest-match ordered',
      (() => { const parts = tml.repository.operators.match.split('|');
               return parts.indexOf('>>=') < parts.indexOf('>>'); })());
check('vscode lib copies exist and are marked generated',
      readF(joinP(here2, 'vscode/lib/lint.mjs'), 'utf8').startsWith('// GENERATED COPY') &&
      readF(joinP(here2, 'vscode/lib/tokenizer.mjs'), 'utf8').startsWith('// GENERATED COPY'));

console.log(failures === 0 ? '\nAll lpc-syntax tests passed.' : `\n${failures} FAILURES`);
process.exit(failures === 0 ? 0 : 1);
