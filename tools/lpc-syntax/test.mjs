// Test suite: node tools/lpc-syntax/test.mjs
// Dependency-free: exercises the tokenizer/highlighter/formatter/linter
// and the generated VS Code assets directly.
import { tokenize, grammar } from './tokenizer.mjs';
import { highlightLPC } from './highlight.mjs';
import { formatLPC, DEFAULT_PRINT_WIDTH, DEFAULT_INDENT_SIZE } from './format.mjs';
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
check('grammar keywords include "struct" (an alternate spelling of L_CLASS,'
      + ' both STRUCT_CLASS and STRUCT_STRUCT are on by default)',
      grammar.keywords.includes('struct') && grammar.keywords.includes('class'));

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
check('numbers: trailing dot, exponent forms, .. stays a range',
      kinds('1. 2.5e2 1e3 2E-5 1..5').join(',') ===
      'number:1.,number:2.5e2,number:1e3,number:2E-5,number:1,operator:..,number:5');
check('numbers: bare "1e" is not an exponent',
      kinds('1e').join(',') === 'number:1,identifier:e');
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
check("directive with an embedded '/* */' comment that closes on a LATER"
      + ' physical line stays ONE directive token (the comment is invisible'
      + " whitespace to the directive, not a separate token that truncates"
      + ' it -- regression for testsuite/single/tests/compiler/preprocessor.lpc'
      + ' silently mis-tokenizing as code after the embedded comment)',
      (() => {
        const src = '#define WARNING_LEVEL 1 /* Change this to higher values to\n'
          + '                           show more warnings. */\n\n// Text after\n';
        const k = kinds(src);
        return k.length === 2 &&
               k[0] === 'directive:#define WARNING_LEVEL 1 /* Change this to higher values to\n'
                 + '                           show more warnings. */' &&
               k[1] === 'comment:// Text after';
      })());
check("directive with an embedded comment that closes and is followed by MORE"
      + " of the directive's own text on that same physical line (e.g. `10 /*"
      + ' spans\\none line */ + 5`) keeps that trailing text as part of the'
      + ' directive, not re-tokenized as code',
      kinds('#define CMT_TAIL 10 /* spans\none line */ + 5\n').length === 1 &&
      kinds('#define CMT_TAIL 10 /* spans\none line */ + 5\n')[0] ===
        'directive:#define CMT_TAIL 10 /* spans\none line */ + 5');
check("a string literal on a directive line (e.g. `#define CMT_STR \"a/*b\"`)"
      + " must not have its own '/*'-shaped content mistaken for a real"
      + ' comment start when scanning for the directive\'s end',
      kinds('#define CMT_STR "a/*b"\n').length === 1 &&
      kinds('#define CMT_STR "a/*b"\n')[0] === 'directive:#define CMT_STR "a/*b"');
check("a trailing '//' comment on a directive line still ends the directive"
      + ' at that physical line as before (unaffected by the embedded-block-'
      + 'comment fix)',
      (() => {
        const k = kinds('#ifdef CREDITS_KEY // trailing comment here too\n#define IFDEF_CMT_OK 1\n#endif\n');
        return k.length === 3 && k.every((t) => t.startsWith('directive:'));
      })());
check("format.mjs never re-spaces a macro argument that's stringized via a"
      + " bare '#param' (`#define STR(x) #x`) -- STR(1+2) must keep"
      + ' stringizing to "1+2", not silently become "1 + 2", since the'
      + ' preprocessor captures the argument\'s exact source spelling'
      + ' (regression: testsuite/single/tests/compiler/preprocessor.lpc)',
      (() => {
        const src = 'void f() {\n#define STR(x) #x\n  ASSERT_EQ("1+2", STR(1+2));\n  ASSERT_EQ("hello", STR(hello));\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check("'##' token paste is NOT stringize -- a macro using only '##' on its"
      + " parameters (`#define GLUE(a, b) a##b`) leaves its call-site"
      + ' arguments free to be reformatted normally, and an unrelated'
      + " sibling '#'-stringize parameter in the SAME macro doesn't leak"
      + ' its protection onto a different, non-stringized parameter',
      (() => {
        const glue = formatLPC('void f() {\n#define GLUE(a, b) a##b\n  int GLUE(fo,o)  =  42;\n}\n');
        const mixed = formatLPC('void f() {\n#define F(a, b) #a b\n  F(1+2,3+4);\n}\n');
        return glue.includes('int GLUE(fo, o) = 42;') &&
               mixed.includes('F(1+2, 3 + 4);');
      })());
check('a macro call nested inside a stringized argument (`STR(STR(1+2))`) is'
      + " frozen as ONE verbatim span covering the WHOLE outer argument,"
      + ' matching real preprocessor semantics (stringize/paste operate on'
      + ' unexpanded argument text) rather than only protecting the inner call',
      (() => {
        const src = 'void f() {\n#define STR(x) #x\n  ASSERT_EQ("STR(1+2)", STR(STR(1+2)));\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('stringize-macro protection UNIONS every definition in the file and'
      + ' ignores #undef/redefinition position: the formatter cannot evaluate'
      + ' preprocessor conditions, and a `#define STR(x) x` redefinition or'
      + ' #undef sitting inside `#if 0` (never executed by the driver) must'
      + ' not strip protection from call sites the live definition still'
      + ' stringizes -- over-masking only preserves spacing, under-masking'
      + ' silently changes program output (driver-verified)',
      (() => {
        const deadRedef = '#define STR(x) #x\n#if 0\n#define STR(x) x\n#endif\nmixed q = STR(1+2);\n';
        const deadUndef = '#define STR(x) #x\n#if 0\n#undef STR\n#endif\nmixed q = STR(1+2);\n';
        const liveRedef = 'void f() {\n#define STR(x) #x\n  a = STR(1+2);\n#define STR(x) (x)\n  b = STR(1+2);\n}\n';
        const o1 = formatLPC(deadRedef), o2 = formatLPC(deadUndef), o3 = formatLPC(liveRedef);
        return o1.includes('STR(1+2)') && formatLPC(o1) === o1 &&
               o2.includes('STR(1+2)') && formatLPC(o2) === o2 &&
               // union => BOTH call sites keep their spelling (safe direction)
               o3.includes('a = STR(1+2);') && o3.includes('b = STR(1+2);') &&
               formatLPC(o3) === o3;
      })());
check('stringize detection mirrors the driver\'s directive preprocessing:'
      + ' a comment between \'#\' and the param (`#/*c*/x`), a backslash'
      + ' continuation between them or inside the parameter list, and an'
      + ' odd \'#\'-run (`###x` = paste-then-stringize) all still protect;'
      + ' an even run (`##x`, plain paste) still does not',
      (() => {
        const o1 = formatLPC('#define SC(x) #/*c*/x\nmixed q = SC(1+2);\n');
        const o2 = formatLPC('#define SL(x) # \\\nx\nmixed q = SL(1+2);\n');
        const o3 = formatLPC('#define SP(a, \\\n            b) #b\nmixed q = SP(1+2, 3+4);\n');
        const o4 = formatLPC('#define TH(a,b) a ###b\nmixed q = TH("x", 1+2);\n');
        const o5 = formatLPC('#define PA(a,b) a ##b\nmixed q = PA(x, 1+2);\n');
        return o1.includes('SC(1+2)') && o2.includes('SL(1+2)') && o3.includes('3+4') &&
               o4.includes('1+2') && o5.includes('1 + 2');
      })());
check('a stringize macro named like a reserved word is still masked at its'
      + ' call sites -- the driver resolves macros BEFORE reserved-word'
      + ' lookup, so `#define string(x) #x` + `string(5+6)` stringizes',
      formatLPC('#define string(x) #x\nmixed q = string(5+6);\n').includes('string(5+6)'));
check('macro-argument boundaries mirror the driver\'s collector, which nests'
      + ' only the \'(\' character: a comma inside `x[...]` really splits'
      + ' driver arguments (so the #b flag lands on `y+3]`), while a comma'
      + ' inside `(: ... :)` does not (its delimiters contain raw parens)',
      (() => {
        const o1 = formatLPC('#define F(a,b) #b\nmixed q = F(x[1+2,y+3], 9);\n');
        const o2 = formatLPC('#define G(a,b) #b\nmixed q = G((: f, 1, 2 :), 3+4);\n');
        return o1.includes('y+3]') && o2.includes('3+4');
      })());
check('a quote in a directive never extends it past its physical line'
      + ' (`#define Q it\'` used to swallow the entire next source line into'
      + ' the directive token; an unterminated \'"\' swallowed everything to'
      + ' the next \'"\' in the file) -- while a `\\` continuation and a'
      + ' multi-line /* */ comment still extend it, and a spliced string'
      + ' (`"abc\\<newline>def"`) still continues',
      (() => {
        const d1 = tokenize("#define Q it'\nint f() { return 1; }\n").filter((t) => t.kind === 'directive');
        const d2 = tokenize('#define BAD "abc\n#define UND(x) #x\nmixed q = UND(3+4);\n').filter((t) => t.kind === 'directive');
        const d3 = tokenize('#define X 1 /* c\nc */ + 2\nint y;\n').filter((t) => t.kind === 'directive');
        const d4 = tokenize('#define S "abc\\\ndef"\nint y;\n').filter((t) => t.kind === 'directive');
        const masked = formatLPC('#define BAD "abc\n#define UND(x) #x\nmixed q = UND(3+4);\n');
        return d1.length === 1 && d1[0].text === "#define Q it'" &&
               d2.length === 2 &&
               d3.length === 1 && d3[0].text.includes('+ 2') &&
               d4.length === 1 && d4[0].text.includes('def"') &&
               masked.includes('UND(3+4)');
      })());
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
check('char literal: multi-digit hex/octal escapes are not truncated',
      kinds("'\\x41' '\\101' '\\n'").join(',') ===
      "char:'\\x41',char:'\\101',char:'\\n'");
check('template interpolation: brace inside a nested string/char/comment does not end it early',
      (() => {
        const k1 = kinds('`x=${ s == "}" }`');
        const k2 = kinds("`c=${ ch == '}' }`");
        const k3 = kinds('`r=${/* } */ x}`');
        return k1[k1.length - 1] === 'template:}`' && k1.includes('string:"}"') &&
               k2[k2.length - 1] === 'template:}`' && k2.includes("char:'}'") &&
               k3[k3.length - 1] === 'template:}`' && k3.includes('comment:/* } */');
      })());

// --- highlighter --------------------------------------------------------------
const html = highlightLPC('int f() { return "hi"; } // done');
check('highlight keyword span', html.includes('<span class="lpc-keyword">return</span>'));
check('highlight string span', html.includes('<span class="lpc-string">&quot;hi&quot;</span>'));
check('highlight comment span', html.includes('<span class="lpc-comment">// done</span>'));
check('html escaped', highlightLPC('if (a < b) x = "<&>";').includes('&lt;&amp;&gt;'));
check('highlight $N closure params get their own class, not lpc-identifier',
      highlightLPC('(: $1 + $2 :)').includes('<span class="lpc-param">$1</span>') &&
      highlightLPC('(: $1 + $2 :)').includes('<span class="lpc-param">$2</span>'));
check('highlight illegal character is flagged, not silently dropped',
      highlightLPC('int x = 1 \u0001;').includes('<span class="lpc-unknown">'));

// --- formatter ----------------------------------------------------------------
// Each if/else branch here is a single-statement one-liner already written
// on one source line, so each stays compact (see the one-liner-block tests
// below) while the OUTER function body still expands (it has a nested
// if/else, disqualifying it from one-lining itself).
const ugly = 'int  f( int x ){if(x>0){return   x;}else{return -x;}}';
const pretty = formatLPC(ugly);
check('a one-line source stays one line (any statement count), normalized'
      + ' spacing only',
      pretty === 'int f(int x) { if (x > 0) { return x; } else { return -x; } }\n');
check('formatter idempotent', formatLPC(pretty) === pretty,
      JSON.stringify({ once: pretty, twice: formatLPC(pretty) }));
const uglyML = 'int  f( int x ){\nif(x>0){return   x;}\nelse{return -x;}\n}\n';
const prettyML = formatLPC(uglyML);
check('a multi-line source expands with indented braces; the else chain'
      + ' cuddles onto the if close (corpus convention)',
      prettyML === 'int f(int x) {\n  if (x > 0) { return x; } else { return -x; }\n}\n' &&
      formatLPC(prettyML) === prettyML);
check('nested blocks (if/else) indent one level per depth, not off-by-one, when'
      + ' each branch has multiple statements (disqualifying one-lining)',
      (() => {
        const src = 'int f(int x) {\nif (x > 0) {\nfoo(); return x;\n} else {\nbar(); return -x;\n}\n}\n';
        const out = formatLPC(src);
        // statement groups the source wrote on one line stay merged
        return out === 'int f(int x) {\n  if (x > 0) {\n    foo(); return x;\n'
          + '  } else {\n    bar(); return -x;\n  }\n}\n' &&
          formatLPC(out) === out;
      })());
check('switch/case body indents under the switch, not at column 0',
      (() => {
        const out = formatLPC('int f(int x) {\nswitch (x) {\ncase 1: return 1;\ndefault: return 0;\n}\n}\n');
        return out.includes('\n  switch (x) {') &&
               out.split('\n').every((l) => l === '' || l === 'int f(int x) {' || l === '}' || l.startsWith('  '));
      })());
const withDirective = formatLPC('#define X 1\n   int f(){return X;}');
check('directives at column 0', withDirective.startsWith('#define X 1\n'));
const withComment = formatLPC('// header\nint g(){return 1;}');
check('standalone comment kept', withComment.startsWith('// header\n'));
check('strings verbatim through formatter',
      formatLPC('string s="a  b";').includes('"a  b"'));
check('array literal braces stay inline and do not affect indent depth',
      (() => {
        const out = formatLPC('void f() { return ({ 1, 2, 3 }); }\n');
        return out.includes('return ({ 1, 2, 3 });') && formatLPC(out) === out;
      })());
check('nested array literal in a call is idempotent',
      (() => {
        const src = 'mixed f() { ASSERT(catch(allocate(5, function(int i) { if (i == 2) error("boom"); return i; }))); }\n';
        const once = formatLPC(src);
        return formatLPC(once) === once;
      })());
check('trailing "//" comment forces a line break before following code',
      (() => {
        const src = 'void f() { if (a) //note\n    return b; else c(); }\n';
        const once = formatLPC(src);
        const twice = formatLPC(once);
        return once === twice && !once.split('\n').some((l) => /\/\/note.+\S/.test(l));
      })());
check('formatter is stable on a source that swallows to EOF (unterminated construct)',
      (() => {
        const once = formatLPC('void f() {\n  "\n}\n');
        return formatLPC(once) === once;
      })());
check('case/default label colon has no leading space (unlike ternary/mapping colons)',
      formatLPC('int f(int x) { switch (x) { case 1: return 1; default: return 0; } }\n')
        .includes('case 1:') &&
      formatLPC('int f(int x) { switch (x) { case 1: return 1; default: return 0; } }\n')
        .includes('default:'));
check('heredoc (@/@@) terminator line follows the source: code the source'
      + " wrote right after the terminator stays glued (`TEXT;` -- the"
      + ' driver rescans the rest of the terminator line as code), and'
      + ' code the source put on the NEXT line keeps its own line',
      (() => {
        const glued = 'void f() {\n  string text = @TEXT\nline one\nTEXT;\n}\n';
        const g = formatLPC(glued);
        const src1 = 'int help() {\n    write( @ENDHELP\nhelp text\nENDHELP\n    );\n    return 1;\n}\n';
        const once1 = formatLPC(src1);
        const gluedCall = 'void f() {\n  write(@ENDHELP\nhelp text\nENDHELP, 1);\n}\n';
        const gc = formatLPC(gluedCall);
        return g === glued && formatLPC(g) === g &&
               once1 === formatLPC(once1) && once1.includes('ENDHELP\n  );\n') &&
               gc === gluedCall && formatLPC(gc) === gc;
      })());
check('multiline array/mapping literals preserve their line breaks instead of collapsing',
      (() => {
        const arr = formatLPC('mixed x = ({\n  1,\n  2,\n});\n');
        const map = formatLPC('mapping x = ([\n  "a": 1,\n  "b": 2,\n]);\n');
        return arr === 'mixed x = ({\n  1,\n  2,\n});\n' && formatLPC(arr) === arr &&
               map === 'mapping x = ([\n  "a": 1,\n  "b": 2,\n]);\n' && formatLPC(map) === map;
      })());
check('single-line array/mapping literals stay one line, and two statements'
      + ' the source wrote on one line stay together',
      (() => {
        const out = formatLPC('mixed x = ({ 1, 2, 3 }); mapping m = ([ "a":1, "b":2 ]);\n');
        return out === 'mixed x = ({ 1, 2, 3 }); mapping m = ([ "a": 1, "b": 2 ]);\n';
      })());
check('indexing and ranges stay tight; varargs/spread keep normal spacing',
      (() => {
        const out = formatLPC('int f() { return a[0] + b[1..2] + c()[0] + e[..<4]; }\n');
        const va = formatLPC('mixed f(int a, ...) { return g(1, 2, ...); }\n');
        return out.includes('a[0]') && out.includes('b[1..2]') && out.includes('c()[0]') &&
               out.includes('e[..<4]') && va.includes('int a, ...') && va.includes('g(1, 2, ...)');
      })());
check('indexing/mapping literals nested inside a multiline array literal do not corrupt bracket tracking',
      (() => {
        const src = 'mixed x = ({\n  a[0],\n  b[1..2],\n  ([ "k": 1 ]),\n});\n';
        const once = formatLPC(src);
        return formatLPC(once) === once && once.includes('a[0],') && once.includes('b[1..2],') &&
               once.includes('([ "k": 1 ]),');
      })());
check('a non-empty array/mapping literal always gets at least one inner space -- a mapping literal'
      + ' nested inside an array literal on one line (a switch/case body idiom) gets both paddings',
      (() => {
        const src = 'void f(mixed *garray_mixed, int i) {\n  switch (i) {\n' +
          '    case 0:\n      garray_mixed += ({ i });\n      break;\n' +
          '    case 5:\n      garray_mixed += ({ ([ i: i ]) });\n      break;\n  }\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out &&
               out.includes('garray_mixed += ({ i });') &&
               out.includes('garray_mixed += ({ ([ i: i ]) });');
      })());

// --- formatter: conventions found via a full-testsuite formatting pass --------
// (all fixed after formatting every testsuite/**/*.lpc and *.c file and diffing
// against real-world convention frequency; see the corpus counts in each fix's
// comment in format.mjs)
check('closure literal delimiters get a space on the inner side, not tight',
      formatLPC('mixed f() { return (:$1+$2:); }\n').includes('(: $1 + $2 :)'));
check("a comma inside a bound-argument functional literal (`(: fn, a, b :)`,"
      + ' the documented `functional_open , arg_list : )` grammar form used'
      + ' throughout the corpus by call_out/input_to/defer/etc.) is NOT'
      + " mistaken for the ENCLOSING call's own argument separator when"
      + ' printWidth forces that call to wrap one argument per line -- '
      + ' `(: fn, a, b :)` must stay together as ONE argument, not get split'
      + " into `(: fn, a,` / `b :)` as if it were two",
      (() => {
        const out = formatLPC('void f() { call_out((: one_arg, 1 :), 3, 4, 5); }\n', { printWidth: 20 });
        return out === 'void f() {\n  call_out(\n    (: one_arg, 1 :),\n    3,\n    4,\n    5\n  );\n}\n' &&
               formatLPC(out, { printWidth: 20 }) === out;
      })());
check('the same bound-argument-functional-literal comma is not mistaken for a'
      + " stringize macro's own argument separator either -- STR((: a, b :))"
      + ' has exactly ONE argument and must freeze the WHOLE thing verbatim',
      (() => {
        const src = 'void f() {\n#define STR(x) #x\n  a = STR((: one_arg, 1 :));\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('a functional literal nested inside another functional literal\'s bound'
      + ' arguments composes correctly under printWidth wrapping (bracket'
      + " depth for '(:'/':)' nests properly, doesn't leak across levels)",
      (() => {
        const out = formatLPC('void f() { call_out((: outer_fn, (: inner_fn, 5 :) :), 3); }\n', { printWidth: 20 });
        return out === 'void f() {\n  call_out(\n    (: outer_fn, (: inner_fn, 5 :) :),\n    3\n  );\n}\n' &&
               formatLPC(out, { printWidth: 20 }) === out;
      })());
check('new() stays tight like catch() -- a call, not a control-flow keyword',
      formatLPC('mixed f() { return new("/obj/x"); }\n').includes('new("/obj/x")') &&
      formatLPC('int f(string s, int a) { sscanf(s, "%d", a); return a; }\n').includes('sscanf(s,') &&
      !formatLPC('mixed f() { return new("/obj/x"); }\n').includes('new ('));
check("new()'s class-member-initializer colon and a mapping's key:value colon are both"
      + ' tight before, spaced after (`x: 7`, `([ i: ft ])`)',
      (() => {
        const out = formatLPC('void f(class Pt ref p) { p = new(class Pt, x:7, y:8); }\n');
        return out.includes('new(class Pt, x: 7, y: 8)') &&
               formatLPC('mixed f(mixed i, mixed ft) { return ([i:ft]); }\n').includes('([ i: ft ])');
      })());
check('a mapping key:value colon is tight before it, unlike a ternary colon appearing as a'
      + " mapping's value (which keeps its own normal spacing) -- both on one line and split"
      + ' across a multi-line literal',
      (() => {
        const oneLine = formatLPC('mapping m = ([ "a": cond ? aa : bb ]);\n');
        const multi = formatLPC('mapping m = ([\n  "a": cond ? aa : bb,\n  "b": 2,\n]);\n');
        return oneLine === 'mapping m = ([ "a": cond ? aa : bb ]);\n' && formatLPC(oneLine) === oneLine &&
               multi === 'mapping m = ([\n  "a": cond ? aa : bb,\n  "b": 2,\n]);\n' &&
               formatLPC(multi) === multi;
      })());
check("a multi-line mapping literal's TRAILING elements -- the ones sharing a"
      + " source line with each other right before the closing ']' (so they"
      + " flush together with `flush()` called AFTER the mapping's own"
      + ' litStack frame is already popped) still get their key:value colons'
      + ' recognized as tight, not just the earlier one-element-per-line ones',
      (() => {
        const src = 'mixed x = ([ "ylabels": ({ "mon" }),\n  "xlabels": ({ "00" }), "cellw": 4\n]);\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('a long single-line call argument containing a mapping literal that only'
      + ' needs printWidth-based splitting (not source-multiline) still gets'
      + " each split-out element's mapping colon recognized as tight, not just"
      + ' inheriting whatever mappingContext the call itself had (false, since'
      + ' the call is not itself inside another mapping)',
      (() => {
        const src = 'void f(object db) {\n  res = db->table("users")->insert(([ "name": "fluffos", "bio": "Actively'
          + ' maintained LPMUD driver (LPC interpreter, MudOS fork)" ]));\n}\n';
        const out = formatLPC(src);
        return formatLPC(out) === out && !out.includes('"name" :') && !out.includes('"bio" :');
      })());
check('bare "::" (parent/efun bypass call, no left-hand qualifier) keeps its'
      + ' normal leading space (comma, return) -- only identifier::/efun::'
      + ' qualified forms are tight before "::"; both stay tight after',
      (() => {
        const a = formatLPC('void f() { ASSERT_EQ(42,::proto_fn()); }\n');
        const b = formatLPC('void f() { return::render(); }\n');
        const c = formatLPC('void f() { ASSERT_EQ("parent",   ::   pwhich()); }\n');
        const d = formatLPC('void f() { efun :: act_mxp(); master  ::  compile_object(); }\n');
        return a.includes('ASSERT_EQ(42, ::proto_fn())') &&
               b.includes('return ::render()') &&
               c.includes('ASSERT_EQ("parent", ::pwhich())') &&
               d.includes('efun::act_mxp();') && d.includes('master::compile_object();');
      })());
check('array-type-suffix * stays tight against the identifier (incl. after &/ref, and'
      + ' in a multi-declarator list), unlike multiplication',
      (() => {
        const out = formatLPC('void f(mixed ref *arr, mixed & *arr2) { }\n' +
          'int *a1 = ({0}), *a2 = ({0});\n' +
          'int f(int a, int b) { return a * b; }\n');
        return out.includes('mixed ref *arr, mixed & *arr2') &&
               out.includes('int *a1 = ({ 0 }), *a2 = ({ 0 });') &&
               out.includes('return a * b;');
      })());
check('unary +/- stays tight against its operand; binary +/- keeps its space',
      (() => {
        const out = formatLPC('int f(int a, int b) { return -1 - -2 + a - b; }\n');
        return out.includes('return -1 - -2 + a - b;');
      })());
check('reverse-index [<n] stays tight on both sides',
      formatLPC('int f(mixed *arr) { return arr[<1]; }\n').includes('arr[<1]') &&
      formatLPC('int f(int a, int b) { return a < b; }\n').includes('a < b'));
check('a same-line trailing comment is not severed onto its own line',
      (() => {
        const src = 'private string csi = "";       // accumulated CSI bytes\nprivate int x = 0;\n';
        const out = formatLPC(src);
        // ...and the source's hand-aligned gap before it is preserved
        // (AlignTrailingComments in spirit; two-space minimum otherwise).
        return out.startsWith('private string csi = "";       // accumulated CSI bytes\n') &&
               formatLPC(out) === out;
      })());
check('case/default body indents one level under the label; stacked fallthrough'
      + ' labels stay aligned with each other',
      (() => {
        const src = 'int f(int x) {\n  switch (x) {\n  case 1:\n  case 2:\n'
          + '    foo();\n    break;\n  }\n}\n';
        const out = formatLPC(src);
        return out === 'int f(int x) {\n  switch (x) {\n    case 1:\n    case 2:\n'
          + '      foo();\n      break;\n  }\n}\n' &&
          formatLPC(out) === out;
      })());
check('nested switch (case body of an outer switch) indents correctly at every level',
      (() => {
        const src = 'void f(int x, int y) {\n  switch (x) {\n  case 1:\n'
          + '    switch (y) {\n    case 2:\n      foo();\n      break;\n    }\n'
          + '    bar();\n    break;\n  }\n}\n';
        const out = formatLPC(src);
        return out === 'void f(int x, int y) {\n  switch (x) {\n    case 1:\n'
          + '      switch (y) {\n        case 2:\n          foo();\n'
          + '          break;\n      }\n      bar();\n      break;\n'
          + '  }\n}\n' &&
          formatLPC(out) === out;
      })());

// --- formatter: configurable indent size ----------------------------------------
check('default indent size is 2', DEFAULT_INDENT_SIZE === 2);
check('indentSize option controls the indent unit (default 2)',
      (() => {
        // Multi-statement if-body (disqualified from one-lining) so both
        // nesting levels are visible and scale with indentSize.
        const src = 'int f(int x) {\nif (x > 0) {\nfoo(); return x;\n}\n}\n';
        const two = formatLPC(src);
        const four = formatLPC(src, { indentSize: 4 });
        const eight = formatLPC(src, { indentSize: 8 });
        return two === 'int f(int x) {\n  if (x > 0) {\n    foo(); return x;\n  }\n}\n' &&
               four === 'int f(int x) {\n    if (x > 0) {\n        foo(); return x;\n    }\n}\n' &&
               eight === 'int f(int x) {\n        if (x > 0) {\n                foo(); return x;\n        }\n}\n' &&
               formatLPC(two) === two && formatLPC(four, { indentSize: 4 }) === four &&
               formatLPC(eight, { indentSize: 8 }) === eight;
      })());

// --- formatter: empty blocks always collapse ------------------------------------
// A block with NOTHING between '{' and '}' (not even a comment) always
// collapses to tight '{}', regardless of source layout or printWidth --
// there's no content whose line-break choice could matter.
check('an empty function body collapses to {} even if the source had the'
      + ' braces on separate lines',
      (() => {
        const out = formatLPC('void create() {\n}\n');
        return out === 'void create() {}\n' && formatLPC(out) === out;
      })());
check('an empty if/else body inside a larger function collapses to {}',
      (() => {
        const out = formatLPC('void f(int x) {\n  if (x) {\n    foo();\n  } else {\n  }\n}\n');
        return out === 'void f(int x) {\n  if (x) {\n    foo();\n  } else {}\n}\n' &&
          formatLPC(out) === out;
      })());
check('an empty block with a comment inside is NOT collapsed (the comment would'
      + ' be lost or corrupt the close into `{ // ... }`)',
      (() => {
        const src = 'void f() {\n  // TODO: implement\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('an empty array/mapping literal stays tight (`({})`, `([])`), matching'
      + ' the pristine corpus\'s dominant convention (roughly 3:1 and 7:1'
      + ' over the padded spelling) -- only a NON-empty literal gets the'
      + ' inner padding space',
      formatLPC('mixed x = ({});\n') === 'mixed x = ({});\n' &&
      formatLPC('mapping x = ([]);\n') === 'mapping x = ([]);\n' &&
      formatLPC(formatLPC('mixed x = ({});\n')) === formatLPC('mixed x = ({});\n'));
check('an empty or whitespace-only source formats to an empty string, not a'
      + ' manufactured newline',
      formatLPC('') === '' &&
      formatLPC('   \n  \n') === '' &&
      formatLPC('\t\n\n') === '' &&
      formatLPC('// just a comment\n') === '// just a comment\n');

// --- formatter: single-line block preservation ---------------------------------
// A `{ ... }` block the source already wrote on one physical line, with
// exactly one statement and no nested control-flow block, stays one line as
// long as it still fits printWidth -- otherwise it falls through to the
// normal one-statement-per-line expansion.
check('a single-statement one-liner function stays compact',
      (() => {
        const out = formatLPC('string which() { return "c"; }\n');
        return out === 'string which() { return "c"; }\n' && formatLPC(out) === out;
      })());
check('a multi-statement one-line body stays one line, exactly as the'
      + ' source wrote it (`void event_ping(object from, int v) { origin ='
      + ' from; value = v + 1; }`)',
      (() => {
        const out = formatLPC('void f() { foo(); bar(); }\n');
        const ep = 'void event_ping(object from, int v) { origin = from; value = v + 1; }\n';
        return out === 'void f() { foo(); bar(); }\n' && formatLPC(out) === out &&
               formatLPC(ep) === ep;
      })());
check("a block that is a call's last argument keeps the call close glued"
      + ' to its brace -- `});` and `} :), 1);`, never a `)` on its own'
      + ' line',
      (() => {
        const fn = 'void f() {\n  async_write("/log", 1, function(int res) {\n    a();\n    b();\n  });\n}\n';
        const fp = 'void f() {\n  call_out((: g, function() {\n    a();\n  } :), 1);\n}\n';
        const o1 = formatLPC(fn), o2 = formatLPC(fp);
        return o1 === fn && formatLPC(o1) === o1 &&
               o2 === fp && formatLPC(o2) === o2;
      })());
check('a one-line body with a nested real block stays one line too --'
      + ' the source wrote the whole thing on one line and it fits',
      (() => {
        const out = formatLPC('void f(int x) { if (x) { foo(); } }\n');
        return out === 'void f(int x) { if (x) { foo(); } }\n' && formatLPC(out) === out;
      })());
check('a one-line body whose single statement is itself a brace-less'
      + ' if/while/for is still "one statement" and stays compact',
      (() => {
        const out = formatLPC('void f(int x) { if (x) return; }\n');
        return out === 'void f(int x) { if (x) return; }\n' && formatLPC(out) === out;
      })());
check('a body already split across multiple source lines is NOT forced onto one'
      + ' line just because it has one statement',
      (() => {
        const src = 'string which() {\n  return "c";\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('a one-liner that would exceed printWidth still expands, even though the'
      + ' source wrote it on one line',
      (() => {
        const src = 'void some_function_with_a_rather_long_name() {'
          + ' return some_other_call(argument_one, argument_two, argument_three, argument_four); }\n';
        const out = formatLPC(src, { printWidth: 60 });
        return out !== src.replace(/ +/g, ' ') && out.includes('{\n') && formatLPC(out, { printWidth: 60 }) === out;
      })());
check('a one-liner function literal nested inside a call is not disqualified by'
      + " its own block (not a DIRECT child of the enclosing statement's block)"
      + ' -- the whole single-statement call stays compact',
      (() => {
        const out = formatLPC('void f(mixed *arr) { sort_array(arr, function(int a, int b) { return a - b; }); }\n');
        return out === 'void f(mixed *arr) { sort_array(arr, function(int a, int b) { return a - b; }); }\n' &&
          formatLPC(out) === out;
      })());
check('same one-liner function literal, now as the second of two statements'
      + ' (disqualifying the outer body): the callback still renders as its own'
      + ' compact one-liner inside the expanded body',
      (() => {
        const src = 'void f(mixed *arr) {\n  foo();\n'
          + '  sort_array(arr, function(int a, int b) { return a - b; });\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());

// --- formatter: multi-line call/condition preservation --------------------------
// A call, condition, or declaration parameter list the source already split
// across multiple lines is NOT collapsed onto one line -- generalizes the
// existing array/mapping-literal multiline tracking to plain '(' groups.
check('a multi-line function call keeps its source line breaks (one-per-line'
      + ' argument layout), closing paren dedented onto its own line',
      (() => {
        const src = 'void f() {\n  foo(a,\n    b,\n    c\n  );\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('a multi-line boolean condition keeps its source line break',
      (() => {
        const src = 'void f() {\n  if (a &&\n    b\n  ) {\n    c();\n  }\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('a call already on one source line still collapses normally (multi-line'
      + ' preservation only kicks in when the source itself broke it)',
      formatLPC('void f() {\n    foo(a, b, c);\n}\n') === 'void f() {\n  foo(a, b, c);\n}\n');
check('a brace-less if body the source wrote on its own line keeps its break'
      + ' (does not collapse onto the condition line)',
      (() => {
        const src = 'void f(int i, string str) {\n  if (i != 100)\n    ASSERT(!test_bit(str, i));\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('a brace-less if body already on the condition\'s line still collapses'
      + ' normally (dangling-body preservation only kicks in when the source'
      + ' itself broke it)',
      formatLPC('void f(int x) { if (x) return foo(); }\n') === 'void f(int x) { if (x) return foo(); }\n');
check('brace-less if/else bodies each on their own line keep their breaks, and'
      + ' an else-if chain composes correctly',
      (() => {
        const src = 'void f() {\n  if (a)\n    do_a();\n  else if (b)\n    do_b();\n  else\n    do_c();\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('nested brace-less if bodies (no real block anywhere) all resolve at the'
      + ' one terminating statement, and a sibling statement after the chain'
      + ' is not left over-indented',
      (() => {
        const src = 'void f() {\n  if (a)\n    if (b)\n      foo();\n  bar();\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('a dangling if whose single statement turns out to be a real if/else'
      + ' block is not left over-indented once the block closes',
      (() => {
        const src = 'void f() {\n  if (a)\n    if (b) {\n      foo();\n    } else {\n      baz();\n    }\n  bar();\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('brace-less while/for/foreach bodies on their own line keep their break',
      (() => {
        const w = formatLPC('void f() {\n  while (x)\n    foo();\n  bar();\n}\n');
        const fo = formatLPC('void f() {\n  for (i = 0; i < 10; i++)\n    foo();\n  bar();\n}\n');
        const fe = formatLPC('void f() {\n  foreach (int i in arr)\n    foo();\n  bar();\n}\n');
        return w.includes('while (x)\n    foo();\n  bar();') &&
               fo.includes('for (i = 0; i < 10; i++)\n    foo();\n  bar();') &&
               fe.includes('foreach (int i in arr)\n    foo();\n  bar();') &&
               formatLPC(w) === w && formatLPC(fo) === fo && formatLPC(fe) === fe;
      })());
check('a do-while loop (one-liner or multi-line) is unaffected by the'
      + " dangling-body tracking (while's own trailing check has no body)",
      (() => {
        const a = formatLPC('void f() {\n  do {\n    foo();\n  } while (x);\n  bar();\n}\n');
        const b = formatLPC('void f() { do { foo(); } while (x); }\n');
        return a === 'void f() {\n  do {\n    foo();\n  } while (x);\n  bar();\n}\n' &&
               b === 'void f() { do { foo(); } while (x); }\n' &&
               formatLPC(a) === a && formatLPC(b) === b;
      })());
check('a multi-line string literal (embedded newlines) does not falsely trigger'
      + " multi-line-call tracking for whatever follows it on the string's own"
      + ' last physical line',
      (() => {
        const src = 'void f() {\n  read_file("" "line one\nline two\n" "");\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('deeply/pathologically nested one-per-line calls (e.g. a "crasher" test'
      + ' probing a call-depth limit) still preserve the line breaks but the'
      + ' indent stops growing past a sanity cap, instead of exploding without'
      + ' bound',
      (() => {
        const N = 200;
        const src = 'void f() {\n    x(\n' + 'catch(\n'.repeat(N) + '1\n' + ')\n'.repeat(N) + '    );\n}\n';
        const out = formatLPC(src);
        const maxIndent = Math.max(...out.split('\n').map((l) => (/^( *)\S/.exec(l) || ['', ''])[1].length));
        return formatLPC(out) === out && maxIndent > 0 && maxIndent <= 4 * 20 &&
          tokenize(out).filter((t) => t.kind !== 'whitespace').length ===
          tokenize(src).filter((t) => t.kind !== 'whitespace').length;
      })());
check('a standalone comment starting a new source line inside an already-'
      + 'multiline array literal gets its own line, not glued onto the tail of'
      + ' the PREVIOUS element (found in testsuite/command/crasher.lpc\'s'
      + ' "/* --- Section: --- */" style headers)',
      (() => {
        const src = 'mixed *x = ({\n  clone_object("a"),\n  find_object("b"),\n\n'
          + '  /* --- Closures: --- */\n  (: sizeof :),\n});\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out &&
          !out.includes('"b"), /*') && out.includes('\n\n  /* --- Closures: --- */\n');
      })());
check('a blank line between two elements of an already-multiline array literal'
      + ' is preserved, not silently eaten',
      (() => {
        const src = 'mixed x = ({\n  1,\n\n  2,\n});\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('a blank line between two arguments of an already-multiline call is'
      + ' preserved, not silently eaten',
      (() => {
        const src = 'void f() {\n  foo(a,\n\n    b\n  );\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());

// --- formatter: printWidth wrapping --------------------------------------------
check('default printWidth is 100 (ColumnLimit in src/.clang-format)', DEFAULT_PRINT_WIDTH === 100);
check('formatLPC with no options: a single-statement one-liner stays compact'
      + ' (default printWidth)',
      formatLPC('int f(int x) { return x + 1; }\n') === 'int f(int x) { return x + 1; }\n');
check('long parameter list + long call wrap one-per-line, closing paren dedented',
      (() => {
        const src = 'void f(int argument_one, int argument_two, string argument_three, mixed argument_four)'
          + ' { return call_something(argument_one, argument_two, argument_three, argument_four); }\n';
        const once = formatLPC(src, { printWidth: 60 });
        const twice = formatLPC(once, { printWidth: 60 });
        return once === twice &&
               once.includes('void f(\n  int argument_one,\n') &&
               once.includes(') {\n') &&
               once.includes('return call_something(\n    argument_one,\n') &&
               once.split('\n').every((l) => l.length <= 60 || !/[,(]/.test(l));
      })());
check('long array literal wraps one element per line via printWidth (not just source gaps)',
      (() => {
        const src = 'mixed x = ({ 111111111, 222222222, 333333333, 444444444, 555555555, 666666666 });\n';
        const once = formatLPC(src, { printWidth: 40 });
        return once === formatLPC(once, { printWidth: 40 }) &&
               once.startsWith('mixed x = ({\n  111111111,\n') &&
               once.trimEnd().endsWith('});');
      })());
check('a long line with no splittable bracket group is left as one line (no unsafe/non-idempotent break)',
      (() => {
        const src = 'int f() { return aaaaaaaaaa + bbbbbbbbbb + cccccccccc + dddddddddd + eeeeeeeeee; }\n';
        const once = formatLPC(src, { printWidth: 20 });
        return once === formatLPC(once, { printWidth: 20 }) &&
               once.includes('return aaaaaaaaaa + bbbbbbbbbb + cccccccccc + dddddddddd + eeeeeeeeee;');
      })());
check('short code is unaffected by a small printWidth when it already fits',
      formatLPC('int x;\n', { printWidth: 40 }) === 'int x;\n');

// --- formatter: adversarial-review regressions (agent-found bug classes) --------
check('token-merge safety net: two tokens are never butted together if their'
      + ' concatenation re-lexes as a different token sequence -- `a - --b`'
      + ' must not become `a ---b` (which re-lexes as `(a--) - b`), `- -x`'
      + ' must not become `--x` (a pre-decrement!), and `f( ::g() )` must'
      + ' not become `f(::g())` (whose `(:` re-lexes as a functional-literal'
      + ' opener); already-tight `i--`/`a[0]`/`-1`/`efun::` forms stay tight',
      (() => {
        const cases = [
          ['x = f( ::g() );\n', 'f( ::g());'],
          ['y = a - --b;\n', 'a - --b'],
          ['y = a + ++b;\n', 'a + ++b'],
          ['y = - -x;\n', '- -x'],
          ['x = i-- - --j;\n', 'i-- - --j'],
        ];
        for (const [src, want] of cases) {
          const out = formatLPC(src);
          if (!out.includes(want) || formatLPC(out) !== out) return false;
        }
        for (const tight of [['i++;\n', 'i++;'], ['x = -1;\n', '= -1'],
                             ['x = efun::sizeof(a);\n', 'efun::sizeof(a)'],
                             ['x = a[0];\n', 'a[0]']]) {
          if (!formatLPC(tight[0]).includes(tight[1])) return false;
        }
        return true;
      })());
check('keyword-tail safety net: a reserved word never has the next token'
      + ' glued onto it (`return !x;` not `return!x`, `return ~x;`,'
      + ' `return -1;`, `case ..0:` not `case..0:` -- the pristine corpus'
      + " spelling), while the legitimate tight followers stay tight"
      + ' (`return;`, `break;`, `default:`, `catch(`, `new(`, `efun::`)',
      (() => {
        const spaced = [
          ['int f() { return!x; }\n', 'return !x;'],
          ['int f() { return~x; }\n', 'return ~x;'],
          ['int f() { return-1; }\n', 'return -1;'],
          ['void f() { switch (x) { case..0: break; } }\n', 'case ..0:'],
        ];
        const tight = [
          ['int f() { return; }\n', 'return;'],
          ['void f() { break; }\n', 'break;'],
          ['void f() { switch (x) { default: break; } }\n', 'default: break;'],
          ['int f() { return catch(g()); }\n', 'catch(g())'],
          ['int f() { return efun::sizeof(a); }\n', 'efun::sizeof(a)'],
        ];
        for (const [src, want] of [...spaced, ...tight]) {
          const out = formatLPC(src);
          if (!out.includes(want) || formatLPC(out) !== out) return false;
        }
        return true;
      })());
check('a trailing line comment stays at the end of its line at EVERY'
      + ' flush site -- after a one-liner block, an empty block, and a'
      + ' multi-line block close -- and its length never triggers'
      + ' printWidth wrapping of the code it annotates',
      (() => {
        const oneLiner = 'void f() {\n  if (lib <= 0) { ASSERT(1); return; }  // FFI unreachable\n  ASSERT(lib > 0);\n}\n';
        const empty = 'void f() {\n  if (x) {}  // note\n  b();\n}\n';
        const close = 'void f() {\n  if (x) {\n    a();\n  }  // trailing on close\n  b();\n}\n';
        const long = 'void f() {\n  g(aa, bb);  // ' + 'x'.repeat(130) + '\n}\n';
        for (const src of [oneLiner, empty, close, long]) {
          const out = formatLPC(src);
          if (out !== src || formatLPC(out) !== out) return false;
        }
        return true;
      })());
check("'(' is tight after anything callable -- an index"
      + ' (`funcs[0](5, 5)`), a call result (`g(a)(b)`), a functional'
      + ' literal (`(: g :)(5)`) -- while a control-flow CONDITION\'s ")"'
      + ' keeps its space before a parenthesized body (`if (x) (a = b);`),'
      + " and a cast's ')' is tight against its operand"
      + ' (`(string)"x"`, `(mixed *)arr` -- 25:1 in the pristine corpus)',
      (() => {
        const cases = [
          'void f() { ASSERT_EQ(10, funcs[0](5, 5)); }\n',
          'void f() { x = g(a)(b); }\n',
          'void f() { x = (: g :)(5); }\n',
          'void f() { if (x) (a = b); }\n',
          'void f() { x = (string)"abc"; y = (object)tp; }\n',
          'void f() { a = (mixed *)arr; }\n',
          'void f(int) {}\n',
        ];
        for (const src of cases) {
          const out = formatLPC(src);
          if (out !== src || formatLPC(out) !== out) return false;
        }
        // the spaced spelling normalizes to tight
        return formatLPC('void f() { g(funcs[1] (5, 2)); }\n') === 'void f() { g(funcs[1](5, 2)); }\n';
      })());
check('clang-format parity (src/.clang-format, Google base): a cast stays'
      + ' tight after a control-flow keyword too (`return (string)x`,'
      + ' 5:0 pristine), including before a parenthesized operand'
      + ' (`return (string)(x + 1)`), while a real condition\'s \')\''
      + ' before \'(\' keeps its space',
      (() => {
        const cases = [
          'mixed f(mixed x) { return (string)x; }\n',
          'mixed f(mixed x) { return (mixed *)x; }\n',
          'mixed f(mixed x) { return (string)(x + 1); }\n',
          'void f() { if (x) (a = b); }\n',
        ];
        for (const src of cases) {
          const out = formatLPC(src);
          if (out !== src || formatLPC(out) !== out) return false;
        }
        return formatLPC('mixed f(mixed x) { return (string) x; }\n') ===
               'mixed f(mixed x) { return (string)x; }\n';
      })());
check('clang-format parity: SpacesBeforeTrailingComments -- a trailing'
      + " '//' comment gets at least two spaces before it (a one-space or"
      + ' zero-space source gap widens to two), a WIDER source gap is'
      + ' preserved exactly (hand-aligned comment columns stay aligned,'
      + ' AlignTrailingComments in spirit), and a mid-line comment with'
      + ' code after it keeps the normal single space',
      (() => {
        if (formatLPC('int f() {\n  return 1; // one\n}\n') !==
            'int f() {\n  return 1;  // one\n}\n') return false;
        if (formatLPC('int f() {\n  return 1;// none\n}\n') !==
            'int f() {\n  return 1;  // none\n}\n') return false;
        const aligned = 'int f() {\n  return 1;    // aligned\n}\n';
        if (formatLPC(aligned) !== aligned) return false;
        const mid = 'int f() {\n  h(/* mid */ 1);  // tail\n}\n';
        return formatLPC(mid) === mid && formatLPC(formatLPC(mid)) === mid;
      })());
check('docs/lpc/style-guide.md contract: every spelling the style guide'
      + ' documents is a fixed point of the formatter (spacing table,'
      + ' brace/switch examples, fully-empty `for (;;)` tight vs'
      + ' partially-empty `for (i = 0; ; i++)`/`for (x = 1; ; )` spaced'
      + ' -- the corpus\'s and clang-format\'s split), so the published'
      + ' docs and the engine cannot drift apart',
      (() => {
        const fixedPoints = [
          'int find(string name) {\n  if (!name) {\n    return -1;\n  } else {\n    return lookup(name);\n  }\n}\n',
          'void f() {\n  do {\n    step();\n  } while (more());\n}\n',
          'void create() {}\n',
          'void f() { if (x) g(); while (x) g(); foreach (v in arr) g(v); }\n',
          'void f() { for (;;) g(); }\n',
          'void f() { for (i = 0; ; i++) g(); }\n',
          'void f() { for (x = 1; ; ) g(); }\n',
          'void f() { for (; i < n; i++) g(); }\n',
          'int f() { return (x); }\n',
          'void f() { write(msg); funcs[0](5, 5); x = f(a)(b); }\n',
          'void f() { err = catch(g()); ob = new("/obj"); }\n',
          'void f() { a = b + c * d; a >>= 2; x = a ? b : c; }\n',
          'void f() { x = -a; y = !ok; z = ~bits; ++i; i++; }\n',
          'void f() { x = (string)x; y = (mixed *)arr; }\n',
          'int f() { return (int)v; }\n',
          'void f() { x = arr[0]; y = m["key"]; z = s[1..<2]; }\n',
          'void f() { ob->query(); efun::write("x"); }\n',
          'string *names;\nint *fn(mixed *args) { return args; }\n',
          'int *a = ({ 1, 2, 3 });\nint *b = ({});\n',
          'mapping m = ([ "a": 1, "b": 2 ]);\nmapping n = ([]);\n',
          'function g = (: f :);\nfunction h = (: $1 + $2 :);\n',
          'void f(int a, string b: (: "x" :)) {}\n',
          'void f() { g(args...); }\nvoid h(int a, ...) {}\n',
          'int f() {\n  return 1;  // why\n}\n',
          'void f(int x) {\n  switch (x) {\n    case 1:\n      handle_one();\n      break;\n    case LOW..HIGH:  // LPC range label\n      handle_range();\n      break;\n    default:\n      handle_rest();\n  }\n}\n',
          'void event_ping(object from, int v) { origin = from; value = v + 1; }\n',
          'int f(int raw) {\n  if (!raw)\n    return mask[0];\n  return 0;\n}\n',
        ];
        for (const src of fixedPoints) {
          const out = formatLPC(src);
          if (out !== src || formatLPC(out) !== out) return false;
        }
        return formatLPC('void f() { for (; ; ) g(); }\n') === 'void f() { for (;;) g(); }\n' &&
               formatLPC('void f() { for (i = 0;; i++) g(); }\n') === 'void f() { for (i = 0; ; i++) g(); }\n';
      })());
check("a default-argument colon is tight after the parameter name"
      + ' (`string b: (: "str" :)` -- every pristine-corpus site), while'
      + ' ternary colons in argument lists keep their space',
      (() => {
        const cases = [
          'void test4(int a, string b: (: "str" :), int c: (: -1 :)) {\n  a = 1;\n}\n',
          'int id(string id: (: "" :)) { return 1; }\n',
          'void f() { g(a ? b : c, d); }\n',
        ];
        for (const src of cases) {
          const out = formatLPC(src);
          if (out !== src || formatLPC(out) !== out) return false;
        }
        return true;
      })());
check('template interpolation hugs its expression -- nothing after `${`'
      + ' or before `}` (`\`${name}\``, tight 23:3 in the pristine'
      + " corpus), while the interpolation's interior keeps normal"
      + ' spacing (`\`${a + b}\``)',
      (() => {
        const tight = 'void f() { result = \`${name}\` \`${count}\`; }\n';
        const multi = 'void f() { r = \`a${x}b${y}c\`; }\n';
        return formatLPC(tight) === tight &&
               formatLPC('void f() { result = \`${ name }\` \`${ count }\`; }\n') === tight &&
               formatLPC('void f() { r = \`x${ a + b }y\`; }\n') === 'void f() { r = \`x${a + b}y\`; }\n' &&
               formatLPC(multi) === multi;
      })());
check('a multi-line string/template/heredoc token is NEVER re-flowed,'
      + ' indented mid-token, or allowed to trigger wrapping of the code'
      + ' around it (its interior length does not count toward'
      + ' printWidth), and a mixed-EOL source is left alone (only a'
      + ' uniformly-CRLF source converts -- otherwise heredoc content'
      + ' would change value)',
      (() => {
        const hd = 'void f() {\n  set_help(one, @END\nline one of help text here\nline two of help text here\nEND, two);\n}\n';
        const ms = 'void f() {\n  s = report(a, "line1\nline2\nline3", b);\n}\n';
        const mixed = 'void f() {\r\n  s = @T\nlfline\nT;\r\n  a();\r\n}\r\n';
        return formatLPC(hd, { printWidth: 30 }) === hd &&
               formatLPC(ms, { printWidth: 25 }) === ms &&
               !formatLPC(mixed).includes('lfline\r\n');
      })());
check('narrow-printWidth wrapping is idempotent even when a wrapped line'
      + ' starts by closing bumped frames from earlier lines (the wrap'
      + " depth model mirrors the main loop's deferred dedents), and a"
      + ' wrap trailer starting with closes still knows its mapping'
      + ' context (`}), "cellw": 4 ])` keeps the key colon tight)',
      (() => {
        const src = 'void f() {\n  foreach (string s in map(filter(g(dir),\n    (: $1 :)),\n    (: $1 :)) - ({ ".", ".." })) {\n    h(s);\n  }\n}\n';
        const o = { printWidth: 20 };
        const p1 = formatLPC(src, o);
        const m = 'mixed x = f(g(([ "rows": ({ "aaaa", "bbbb", "cccc" }), "cellw": 4 ])));\n';
        const q1 = formatLPC(m, { printWidth: 20 });
        return formatLPC(p1, o) === p1 &&
               formatLPC(q1, { printWidth: 20 }) === q1 && q1.includes('"cellw": 4');
      })());
check("'...' hugs the expression it spreads (`mixed extra...`,"
      + ' `g(args...)`, `g(({ 1 })...)` -- tight 79:6 in the pristine'
      + ' corpus), while the bare varargs marker after a comma keeps its'
      + ' space (`f(int a, ...)`)',
      (() => {
        const cases = [
          'varargs int vsum(int a, int b, mixed extra...) { return 1; }\n',
          'void f() { g(args...); }\n',
          'void f() { g(({ 1 })...); }\n',
          'mixed f(int a, ...) { return g(1, 2, ...); }\n',
        ];
        for (const src of cases) {
          const out = formatLPC(src);
          if (out !== src || formatLPC(out) !== out) return false;
        }
        return formatLPC('varargs int v(mixed extra ...) { return 1; }\n')
          === 'varargs int v(mixed extra...) { return 1; }\n';
      })());
check('final-review regressions: char-escape literals inside template'
      + ' interpolations survive intact (skipCharSpan scans to the closing'
      + " quote -- `\`${'\\x41'}\`` was torn apart); comments/directives"
      + ' inside an interpolation never grow phantom blank lines across'
      + ' passes; a comma-expression subscript is never wrap-split; a'
      + ' one-lined block is never split internally by printWidth; glued'
      + ' content before a switch close keeps body depth; wrap depth is'
      + ' capped at MAX_MULTILINE_INDENT like the main loop',
      (() => {
        const idem = (src, o) => { const p = formatLPC(src, o); return formatLPC(p, o) === p; };
        const f1 = "s = \`${'\\x41'}\`;\n";
        if (formatLPC(f1) !== f1) return false;
        const f3 = 's = \`${ // a\n// b\n}\`;\n';
        const g1 = formatLPC(f3), g2 = formatLPC(g1);
        if (g2 !== formatLPC(g2)) return false;
        return idem('void t() {\n  v = B(x[1 + 2, y + 3]);\n}\n', { printWidth: 20 }) &&
               idem('s = f(function() { return g(aaaa, bbbb); }) + "tail string padding padding'
                 + ' padding padding padding padding padding padding out";\n') &&
               idem('void f() {\n  switch (x) {\n    case 1:\n      b;\n      /* c */}\n}\n') &&
               idem('x = f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(g("aa", 2))))))))))))))))));\n', { printWidth: 20 });
      })());
check('a ternary whose ? and : land on different physical lines (via a'
      + ' printWidth split or a source break) keeps its colon spaced'
      + ' inside a mapping element on every pass -- pending-? counts are'
      + ' tracked per frame across lines',
      (() => {
        const o = { printWidth: 20 };
        const f2 = 'mixed m = ([ "k": cond ? funcall(a, b) : z,\n  "j": 2 ]);\n';
        const p1 = formatLPC(f2, o);
        const p9 = 'void f() {\n  m = ([\n    "k": cond ?\n      aaa : bbb,\n    "j": 2,\n  ]);\n}\n';
        const q1 = formatLPC(p9);
        return formatLPC(p1, o) === p1 && q1.includes('aaa : bbb') && formatLPC(q1) === q1;
      })());
check('fast-path block closes stay aligned with the deferred-dedent queue'
      + ' (an anonymous `function() {}` argument before a wide wrapped'
      + ' call no longer flips its indent between passes)',
      (() => {
        const o = { printWidth: 40 };
        const src = 'void t() {\n  x = foo(bar,\n      function() {}, baz) + wide(aaaa, bbbb, cccc, dddd, eeee);\n}\n';
        const p1 = formatLPC(src, o);
        return formatLPC(p1, o) === p1;
      })());
check('conventions from the final pristine audit: empty for-header clauses'
      + ' keep their space (`for (i = 0; ; i++)`), a cast is tight before a'
      + ' call-like keyword (`(string)new(...)`), `$(width)` is tight, an'
      + " index close before an array literal's close keeps the inner pad"
      + ' (`s[0] });`), and a bare top-level macro line keeps its blank-line'
      + ' separation from the next declaration',
      (() => {
        const marco = '#define LONG int f() { return 1; }\nLONG\n\nvoid do_tests() {\n  g();\n}\n';
        const brk = 'void f() {\n  arr += ({ " " + s[0] });\n}\n';
        return formatLPC('void f() { for (i = 0; ; i++) g(); }\n') === 'void f() { for (i = 0; ; i++) g(); }\n' &&
               formatLPC('void f() { x = (string)new("/obj"); }\n').includes('(string)new(') &&
               formatLPC('void f() { x = $(width); }\n').includes('$(width)') &&
               formatLPC(brk) === brk &&
               formatLPC(marco) === marco && formatLPC(formatLPC(marco)) === formatLPC(marco);
      })());
check('blank-line RUNS follow the source exactly -- a two-blank separator'
      + ' stays two blanks (call_out.lpc), a single blank stays single;'
      + ' trailing blanks at EOF are still trimmed',
      (() => {
        const two = 'void f() {\n  mixed calls, call;\n\n\n  if (busy) {\n    write("x");\n  }\n}\n';
        const one = 'int a;\n\nint b;\n';
        const trail = formatLPC('int a;\n\n\n');
        return formatLPC(two) === two && formatLPC(formatLPC(two)) === formatLPC(two) &&
               formatLPC(one) === one && trail === 'int a;\n';
      })());
check('same-source-line statement groups are preserved anywhere, not just'
      + ' on case lines (`a &= 3;  ASSERT_EQ(2, a);` stays one aligned'
      + ' line, single-spaced), while statements on separate source lines'
      + ' keep their own lines',
      (() => {
        const grouped = 'void f() {\n  a &= 3; ASSERT_EQ(2, a);\n  a = 0; a ||= 9; ASSERT_EQ(9, a);\n}\n';
        const split = 'void f() {\n  a &= 3;\n  ASSERT_EQ(2, a);\n}\n';
        return formatLPC(grouped) === grouped && formatLPC(split) === split;
      })());
check('a multi-line string continuation at statement level (adjacent'
      + ' literals or a `+` tail) keeps its source line breaks, indented'
      + ' one level, instead of collapsing onto one line',
      (() => {
        const juxt = 'void f() {\n  src = "a\\n"\n    "b\\n"\n    "c\\n";\n}\n';
        const plus = 'void f() {\n  s = "aaa" +\n    "bbb" +\n    "ccc";\n}\n';
        return formatLPC(juxt) === juxt && formatLPC(formatLPC(juxt)) === formatLPC(juxt) &&
               formatLPC(plus) === plus && formatLPC(formatLPC(plus)) === formatLPC(plus);
      })());
check('template-literal continuations get the same treatment: adjacent'
      + ' templates (whole or ending an interpolated one) across lines'
      + " keep their breaks, a template's own INTERNAL interpolation"
      + ' tokens never trigger it, and mixed string/template pieces work',
      (() => {
        const cases = [
          'void f() {\n  result = `hello`\n    `world`;\n}\n',
          'void f() {\n  result = `a${name}b`\n    `cd`;\n}\n',
          'void f() {\n  result = "hi "\n    `${name}!`;\n}\n',
          'void f() {\n  s = `a` +\n    `b`;\n}\n',
        ];
        for (const src of cases) {
          if (formatLPC(src) !== src || formatLPC(formatLPC(src)) !== formatLPC(src)) return false;
        }
        // internal interpolation still collapses (general expression rule),
        // and now hugs its expression too
        const internal = formatLPC('void f() {\n  r = `x${ a\n  + b }y`;\n}\n');
        return internal === 'void f() {\n  r = `x${a + b}y`;\n}\n';
      })());
check('a string-concat chain may include bare identifiers (string-valued'
      + ' macros, the color-macro idiom `"..." HIY\\n "..."`) and a leading'
      + ' `+` on the continuation line -- breaks inside the chain are'
      + ' preserved until the ";"',
      (() => {
        const macroChain = 'void f() {\n  write("a"\n    NOR\n    "b" +\n    HIY\n    "c");\n}\n';
        const leadingPlus = 'void f() {\n  s = "aaa"\n    + "bbb"\n    + "ccc";\n}\n';
        const m = formatLPC(macroChain);
        const l = formatLPC(leadingPlus);
        return formatLPC(m) === m && l === leadingPlus && formatLPC(l) === l;
      })());
check('a multi-line declarator list with no bracket frame keeps its break'
      + ' after the trailing comma (`int a, b,\\n  c;`), indented one level',
      (() => {
        const src = 'void f() {\n  int empty300, empty1000, empty10000,\n    empty100000, empty200000;\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('a trailing comment on a brace-less condition line stays there'
      + ' (`if (cond)  // note` keeps the note at line end, not orphaned'
      + " onto the body's line)",
      (() => {
        const src = 'int f(int raw) {\n  if (!raw && sizeof(mask))  // note\n    return mask[0];\n  return 0;\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('a CRLF source stays CRLF (at_block_crlf.lpc deliberately exercises'
      + " the driver's CRLF handling; kept dos-style by .gitattributes)",
      (() => {
        const crlf = 'void f() {\r\n  int x = 1;\r\n}\r\n';
        const out = formatLPC(crlf);
        return out === crlf && formatLPC(out) === out &&
               formatLPC('int x;\n') === 'int x;\n';
      })());
check("a multiline construct's CLOSE follows the source like every other"
      + ' line break: written on the last element\'s line it stays glued'
      + ' (`..."\\n");`, `2 ]);` -- with the mapping colon still tight and'
      + ' the literal\'s inner pad kept), written on its own line it keeps'
      + ' it; nested glued closes keep the pristine corpus\'s inter-pair'
      + ' padding (`2 }) ]) });`)',
      (() => {
        const glued = 'void f() {\n  write_src(DIR + "/loop.lpc",\n    "line1" +\n    "line2");\n}\n';
        const map = 'mapping m = ([ "a": 1,\n  "b": 2 ]);\n';
        const nested = 'mixed x = ({ ([ "q": ({ 1,\n  2 }) ]) });\n';
        const ownLine = 'void f() {\n  g(\n    a,\n    b\n  );\n}\n';
        for (const src of [glued, map, nested, ownLine]) {
          const out = formatLPC(src);
          if (out !== src || formatLPC(out) !== out) return false;
        }
        return true;
      })());
check("a modifier-section label's colon is tight (`public:`, `nomask"
      + " private:` -- grammar.y's `type_modifier_list ':'`), and the"
      + " label follows the source's line choice: on its own line it"
      + ' keeps it (pristine-corpus layout), inline it stays merged',
      (() => {
        const inline = formatLPC('private public : void test2() {}\n');
        const ownLine = formatLPC('public:\n\nvoid test() {\n}\n');
        return inline === 'private public: void test2() {}\n' &&
               formatLPC(inline) === inline &&
               ownLine === 'public:\n\nvoid test() {}\n' &&
               formatLPC(ownLine) === ownLine;
      })());
check("'++'/'--' spacing is POSITIONAL, not blanket: tight-before only in"
      + ' postfix position (`i++`, `a[0]--`), spaced-before in prefix'
      + ' position (`return ++count;` must not glue into `return++count`,'
      + ' `x = ++i;` not `x =++i`), and after a POSTFIX one the next token'
      + ' keeps its space (`i++ < n`, `i++ + j`, `i-- - --j`)',
      (() => {
        const cases = [
          ['int bump() { return++count; }\n', 'return ++count;'],
          ['void f() { x = ++i; }\n', 'x = ++i;'],
          ['void f() { for (i = 0; ; ++i) b(); }\n', '; ; ++i)'],
          ['void f() { i++; }\n', 'i++;'],
          ['void f() { a[0]--; }\n', 'a[0]--;'],
          ['void f() { while (i++ < n) b(); }\n', 'i++ < n'],
          ['void f() { x = i++ + j; }\n', 'i++ + j'],
          ['void f() { x = i-- - --j; }\n', 'i-- - --j'],
          ['void f() { f(i++, --j, ++k); }\n', 'f(i++, --j, ++k)'],
        ];
        for (const [src, want] of cases) {
          const out = formatLPC(src);
          if (!out.includes(want) || formatLPC(out) !== out) return false;
        }
        return true;
      })());
check('a brace-less if body inside an anonymous function passed as a call'
      + " argument drains its indent bump at the body's own ';' -- the old"
      + ' parenLevel-gated drain leaked one indent level per occurrence,'
      + ' sinking every later top-level function deeper and spuriously'
      + ' dedenting a later unrelated statement at the same nesting depth',
      (() => {
        const src = 'void f() {\n  call_out(function() {\n    if (a)\n      b();\n  }, 1);\n}\n\nvoid tail() { u(); }\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check("statements inside an anonymous function nested in call arguments get"
      + " one line each -- a ';' directly inside a real block flushes even"
      + ' at parenLevel > 0 (the old gate glued `a(); b(); c();` onto one'
      + ' line inside every callback body)',
      (() => {
        const src = 'void f() {\n  map(arr, function(int x) {\n    a();\n    b();\n  });\n}\n';
        const out = formatLPC(src);
        return out.includes('    a();\n    b();\n') && formatLPC(out) === out;
      })());
check('a mapping key colon on a merged case line whose value continues on'
      + ' the next source line does not stamp a case-body indent bump onto'
      + ' the mapping frame (old behavior: permanent +1 drift for the rest'
      + ' of the file)',
      (() => {
        const src = 'void f() {\n  switch (x) {\n    case 1: m = ([ "a":\n      2\n    ]); break;\n    case 2:\n      b();\n      break;\n  }\n  tail();\n}\n\nvoid g() { u(); }\n';
        const out = formatLPC(src);
        return out.includes('\n  tail();\n') && out.includes('\nvoid g() { u(); }\n') &&
               formatLPC(out) === out;
      })());
check('an `else` after a brace-less chain re-indents to the `if` it actually'
      + ' binds to (the nearest one), not the outermost: `if (a) if (b) c();'
      + ' else d();` written multi-line keeps the else under the INNER if',
      (() => {
        const src = 'void f() {\n  if (a)\n    if (b)\n      c();\n    else\n      d();\n}\n';
        const out = formatLPC(src);
        return out === src && formatLPC(out) === out;
      })());
check('a bare block (or if body) followed by a separate `while` loop is not'
      + ' glued into a pseudo do-while `} while (b) {` -- only a real `do`'
      + " body's close keeps a following `while` on the line",
      (() => {
        const split = formatLPC('void f() {\n  {\n    x();\n  }\n  while (b) {\n    y();\n  }\n}\n');
        const dow = formatLPC('void f() {\n  do {\n    x();\n  } while (b);\n  do {} while (c);\n}\n');
        return !split.includes('} while') && split.includes('}\n  while (b) {') &&
               dow.includes('} while (b);') && dow.includes('do {} while (c);');
      })());
check('a ternary colon on a merged case line stays spaced while the label'
      + ' colon stays tight -- including a label deep in a one-lined switch'
      + ' (`switch (x) { case 1: break; }`), which the old first-token-on-'
      + 'the-line check missed',
      (() => {
        const merged = formatLPC('void f() {\n  switch (x) {\n  case 1: y = a ? b : c; break;\n  }\n}\n');
        const oneLined = formatLPC('void f() { switch (x) { case 1: break; } }\n');
        return merged.includes('case 1: y = a ? b : c; break;') &&
               oneLined.includes('case 1: break;');
      })());
check('a printWidth-wrapped mapping element whose value has its own'
      + ' multiline construct keeps its key colon TIGHT and its ternary'
      + ' colon SPACED on every pass (flush judges mappingContext at the'
      + " element's start frame; wrap trailer slices inherit the header's"
      + ' pending-ternary count)',
      (() => {
        const o = { printWidth: 30, indentSize: 4 };
        const src = 'void f() {\n  m = ([\n    "kkkkkkkk": funcall(aaaaaaaa, bbbbbbbb) ? yyyyyyy : zzzzzzz,\n    "second": 2,\n  ]);\n}\n';
        const p1 = formatLPC(src, o);
        return p1 === formatLPC(p1, o) &&
               p1.includes('"kkkkkkkk": funcall(') && p1.includes(') ? yyyyyyy : zzzzzzz,');
      })());
check('a leading comment glued to the next token on its source line stays'
      + ' glued through printWidth wrapping (`/*x*/ 2_000_000,` as one'
      + ' element line) -- splitting them made wrapped literals'
      + ' non-idempotent',
      (() => {
        const src = 'mixed a = ({ 1_000_000, /*x*/ 2_000_000, 3_000_000 });\n';
        const p1 = formatLPC(src, { printWidth: 40 });
        return p1 === formatLPC(p1, { printWidth: 40 }) && p1.includes('/*x*/ 2_000_000,');
      })());

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
check('tmLanguage: class/struct get their own storage.type.class scope, not keyword.control',
      tml.repository['class-keyword'].name === 'storage.type.class.lpc' &&
      /\bclass\b/.test(tml.repository['class-keyword'].match) &&
      /\bstruct\b/.test(tml.repository['class-keyword'].match) &&
      !/\bclass\b/.test(tml.repository.keywords.match) &&
      !/\bstruct\b/.test(tml.repository.keywords.match));
check('tmLanguage: function-call excludes reserved words (no "if (" misfire as entity.name.function)',
      (() => {
        const re = new RegExp(tml.repository['function-call'].match);
        return !re.test('if (') && !re.test('while (') && !re.test('new (') &&
               !re.test('catch(') && re.test('foo (') && re.test('bar(');
      })());
check('tmLanguage: operators longest-match ordered',
      (() => { const parts = tml.repository.operators.match.split('|');
               return parts.indexOf('>>=') < parts.indexOf('>>'); })());
check('vscode lib copies are marked generated AND byte-identical to their'
      + ' parents after the 3-line header (a stale copy fails the suite,'
      + ' not just a missing banner)',
      ['lint.mjs', 'tokenizer.mjs', 'format.mjs'].every((name) => {
        const copy = readF(joinP(here2, 'vscode/lib/' + name), 'utf8');
        const parent = readF(joinP(here2, name), 'utf8');
        return copy.startsWith('// GENERATED COPY') &&
               copy.split('\n').slice(3).join('\n') === parent;
      }));
check('extension.js wires up the formatter',
      (() => { const src = readF(joinP(here2, 'vscode/extension.js'), 'utf8');
               return src.includes('registerDocumentFormattingEditProvider') &&
                      src.includes('formatLPC'); })());
const langConfig = JSON.parse(readF(joinP(here2, 'vscode/language-configuration.json'), 'utf8'));
check('language-configuration: brackets + doc-comment continuation wired',
      langConfig.brackets.length === 3 &&
      Array.isArray(langConfig.onEnterRules) && langConfig.onEnterRules.length > 0);

console.log(failures === 0 ? '\nAll lpc-syntax tests passed.' : `\n${failures} FAILURES`);
process.exit(failures === 0 ? 0 : 1);
