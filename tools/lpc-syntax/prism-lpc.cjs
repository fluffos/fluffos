// Prism language definition for FluffOS LPC, built from lpc-grammar.json
// so keywords / types / modifiers / operators cannot drift from the compiler.
//
// Usage (Docusaurus / any Prism host):
//   globalThis.Prism = PrismObject;
//   require('./prism-lpc.cjs');
// or:
//   const { registerLPC } = require('./prism-lpc.cjs');
//   registerLPC(PrismObject);

'use strict';

// require() so Docusaurus / Rspack can bundle this for the browser.
// Do not use node:fs here — prism-include-languages runs on the client.
const grammar = require('./lpc-grammar.json');

function escapeRe(s) {
  return s.replace(/[\\^$.*+?()[\]{}|]/g, '\\$&');
}

function wordAlt(words) {
  return [...words]
    .sort((a, b) => b.length - a.length || a.localeCompare(b))
    .map(escapeRe)
    .join('|');
}

const CLASS_LIKE = new Set(['class', 'struct']);

function buildLpcLanguage() {
  const keywords = grammar.keywords.filter((w) => !CLASS_LIKE.has(w));
  const classLike = grammar.keywords.filter((w) => CLASS_LIKE.has(w));
  const reserved = [
    ...grammar.keywords,
    ...grammar.typeKeywords,
    ...grammar.modifierKeywords,
  ];
  const operators = grammar.operators.map(escapeRe).join('|');

  const interpolation = {
    pattern: /((?:^|[^\\])(?:\\{2})*)\$\{(?:[^{}]|\{[^{}]*\})*\}/,
    lookbehind: true,
    inside: {
      punctuation: /^\$\{|\}$/,
    },
  };

  const lpc = {
    comment: [
      { pattern: /\/\*[\s\S]*?(?:\*\/|$)/, greedy: true },
      { pattern: /(^|[^\\:])\/\/.*/, lookbehind: true, greedy: true },
    ],
    directive: {
      pattern: /(^[\t ]*)#(?:[^\n\\]|\\\n|\\(?!\n))*/m,
      lookbehind: true,
      greedy: true,
      alias: 'property',
      inside: {
        'directive-name': {
          pattern: new RegExp(
            `^#\\s*(?:${wordAlt(grammar.directives)})\\b`,
          ),
          alias: 'keyword',
        },
        comment: [
          { pattern: /\/\*[\s\S]*?\*\//, greedy: true },
          { pattern: /\/\/.*/, greedy: true },
        ],
        string: { pattern: /"(?:\\.|[^"\\])*"/, greedy: true },
      },
    },
    textblock: {
      pattern: /^(@@?)([A-Za-z_][A-Za-z0-9_]*)\n[\s\S]*?^\2(?![A-Za-z0-9_])/m,
      greedy: true,
      alias: 'string',
    },
    template: {
      pattern: /`(?:\\[\s\S]|[^\\`])*`/,
      greedy: true,
      alias: 'string',
      inside: {
        interpolation,
        escape: /\\./,
      },
    },
    string: {
      pattern: /"(?:\\.|[^"\\])*"/,
      greedy: true,
    },
    char: {
      pattern: /'(?:\\.|[^'\\])'/,
      greedy: true,
      alias: 'string',
    },
    'class-name': classLike.length
      ? new RegExp(`\\b(?:${wordAlt(classLike)})\\b`)
      : undefined,
    keyword: new RegExp(`\\b(?:${wordAlt(keywords)})\\b`),
    builtin: new RegExp(`\\b(?:${wordAlt(grammar.modifierKeywords)})\\b`),
    'type-keyword': {
      pattern: new RegExp(`\\b(?:${wordAlt(grammar.typeKeywords)})\\b`),
      alias: 'class-name',
    },
    constant: new RegExp(`\\b(?:${wordAlt(grammar.builtinMacros)})\\b`),
    number:
      /\b(?:0[xX][0-9A-Fa-f_]+|0[bB][01_]+|[0-9][0-9_]*(?:\.(?!\.)[0-9_]*)?(?:[eE][+-]?[0-9][0-9_]*)?)(?![A-Za-z0-9_])/,
    function: new RegExp(
      `\\b(?!(?:${wordAlt(reserved)})\\b)[A-Za-z_][A-Za-z0-9_]*(?=\\s*\\()`,
    ),
    variable: /\$[0-9]+/,
    functional: {
      pattern: /\(:|\:\)/,
      alias: 'operator',
    },
    operator: new RegExp(operators),
    punctuation: /[{}[\];(),.:?]/,
  };

  if (!lpc['class-name']) delete lpc['class-name'];
  return lpc;
}

function registerLPC(Prism) {
  if (!Prism || !Prism.languages) {
    throw new Error('registerLPC(Prism) needs a Prism object with languages');
  }
  const lpc = buildLpcLanguage();
  Prism.languages.lpc = lpc;
  Prism.languages.LPC = lpc;
  return lpc;
}

if (typeof globalThis !== 'undefined' && globalThis.Prism) {
  registerLPC(globalThis.Prism);
}

module.exports = {
  grammar,
  buildLpcLanguage,
  registerLPC,
};
