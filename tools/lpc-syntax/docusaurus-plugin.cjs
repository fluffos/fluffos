// Remark helper for FluffOS LPC highlighting on Docusaurus sites.
//
//   docs: {
//     remarkPlugins: [
//       require('../tools/lpc-syntax/docusaurus-plugin.cjs'),
//     ],
//   }
//
// Prism registration still happens via a swizzled prism-include-languages
// (Docusaurus has no plugin hook for Prism languages). See
// docs/src/theme/prism-include-languages.js.

'use strict';

const DEFAULT_KEEP_C = [/[/\\]driver[/\\]/, /build-wasm/];

function walk(node, visit) {
  visit(node);
  const kids = node && node.children;
  if (Array.isArray(kids)) {
    for (const child of kids) walk(child, visit);
  }
}

function remarkLpcFences(options = {}) {
  const keepC = options.keepCAsC || DEFAULT_KEEP_C;
  return (tree, file) => {
    const fp = String((file && (file.path || (file.history && file.history[0]))) || '');
    if (keepC.some((re) => re.test(fp))) return;
    walk(tree, (node) => {
      if (node && node.type === 'code' && (node.lang === 'c' || node.lang === 'C')) {
        node.lang = 'lpc';
      }
    });
  };
}

remarkLpcFences.remarkLpcFences = remarkLpcFences;
remarkLpcFences.DEFAULT_KEEP_C = DEFAULT_KEEP_C;

module.exports = remarkLpcFences;
