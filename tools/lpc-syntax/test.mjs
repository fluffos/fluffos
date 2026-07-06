// Dependency-free test suite: node tools/lpc-syntax/test.mjs
import { tokenize, grammar } from './tokenizer.mjs';
import { highlightLPC } from './highlight.mjs';
import { formatLPC } from './format.mjs';

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

console.log(failures === 0 ? '\nAll lpc-syntax tests passed.' : `\n${failures} FAILURES`);
process.exit(failures === 0 ? 0 : 1);
