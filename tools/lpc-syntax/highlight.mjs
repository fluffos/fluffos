// HTML syntax highlighter over the grammar-driven tokenizer.
// highlightLPC(source) -> HTML string of <span class="lpc-KIND"> runs.

import { tokenize } from './tokenizer.mjs';

const CSS_CLASS = {
  comment: 'lpc-comment',
  directive: 'lpc-directive',
  string: 'lpc-string',
  template: 'lpc-string',
  textblock: 'lpc-string',
  char: 'lpc-string',
  number: 'lpc-number',
  keyword: 'lpc-keyword',
  efunkw: 'lpc-keyword',
  type: 'lpc-type',
  modifier: 'lpc-modifier',
  identifier: 'lpc-identifier',
  operator: 'lpc-operator',
  punctuation: 'lpc-punctuation',
  functional: 'lpc-functional',
};

const escapeHtml = (s) =>
  s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
   .replace(/"/g, '&quot;');

export function highlightLPC(source) {
  let out = '';
  for (const t of tokenize(source)) {
    const cls = CSS_CLASS[t.kind];
    if (cls === undefined || t.kind === 'whitespace') {
      out += escapeHtml(t.text);
    } else {
      out += `<span class="${cls}">${escapeHtml(t.text)}</span>`;
    }
  }
  return out;
}

// Default stylesheet consumers may embed (kept minimal on purpose).
export const defaultCss = `
.lpc-comment { color: #6a737d; font-style: italic; }
.lpc-directive { color: #b30086; }
.lpc-string { color: #22863a; }
.lpc-number { color: #005cc5; }
.lpc-keyword { color: #d73a49; font-weight: 600; }
.lpc-type { color: #6f42c1; }
.lpc-modifier { color: #6f42c1; font-style: italic; }
.lpc-operator { color: #444; }
.lpc-functional { color: #e36209; font-weight: 600; }
`;
