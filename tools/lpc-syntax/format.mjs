// Basic LPC formatter over the grammar-driven tokenizer: brace-depth
// reindentation, single-space operator normalization, directives at
// column 0, strings/comments/text blocks verbatim. Deterministic and
// idempotent (format(format(x)) === format(x) -- pinned by test.mjs).

import { tokenize } from './tokenizer.mjs';

// 100 matches ColumnLimit in src/.clang-format -- the repo's C++ style
// (Google base, IndentWidth 2) that this formatter mirrors for every
// language-common rule.
export const DEFAULT_PRINT_WIDTH = 100;
export const DEFAULT_INDENT_SIZE = 2;
// Sanity cap on how many indent levels the source-line-break-preservation
// mechanism (below) will stack up. Real code never gets close to this;
// it exists for adversarial input (e.g. a "crasher" test nesting 100+
// single-argument calls one per line to probe the driver's own call-depth
// limit), which would otherwise grow the indent without bound.
const MAX_MULTILINE_INDENT = 16;

export function formatLPC(source, options = {}) {
  const printWidth = options.printWidth > 0 ? options.printWidth : DEFAULT_PRINT_WIDTH;
  const indentUnit = ' '.repeat(options.indentSize > 0 ? options.indentSize : DEFAULT_INDENT_SIZE);
  const rawToks = tokenize(source).filter((t) => t.kind !== 'whitespace');
  const toks = maskStringizeArguments(rawToks, source, collectStringizeMacros(rawToks));
  const lines = [];
  let cur = [];
  let depth = 0;
  // Pending indent bumps owed to a BRACE-LESS if/while/for/foreach body or
  // else-body that the source wrote on its own line (e.g.
  // `if (x)\n  stmt();`) -- mirrors the array/mapping/call multiline
  // tracking above, but for control-flow bodies that have no bracket pair
  // of their own to hang the tracking on. Each entry records `litStack.
  // length` at push time (`lit`) so a chain of dangling bodies with no
  // real block in between (`if (a)\n  if (b)\n    stmt();`) all resolve
  // together at the one statement/block that finally terminates the chain
  // (matching `litStack.length` at the ';' or block-close, see below) -- a
  // brace-less body is grammatically always exactly one (possibly itself
  // brace-less) statement, but if that statement turns out to be a REAL
  // `{...}` block, entries pushed before it must wait for the block's own
  // close, not the first ';' inside it. `kw` records WHICH keyword owed
  // the bump ('if'/'while'/'for'/'foreach'/'else') so a following `else`
  // can resolve down to exactly the `if` it binds to (see the ';' branch).
  const danglingBodyStack = [];
  const popDanglingWhile = (pred) => {
    while (danglingBodyStack.length && pred(danglingBodyStack[danglingBodyStack.length - 1])) {
      danglingBodyStack.pop();
      depth = Math.max(0, depth - 1);
    }
  };
  const drainDanglingBody = () => popDanglingWhile((e) => e.lit === litStack.length);
  // Entries recorded at a litStack depth we've already popped BELOW can
  // never `===`-match again (their frame closed without the body's own
  // ';' ever draining them). Without pruning, such a stale entry both
  // leaks its depth bump permanently AND spuriously drains later at some
  // unrelated statement that happens to reach the same litStack depth.
  // Called right after every litStack.pop().
  const pruneDanglingBody = () => popDanglingWhile((e) => e.lit > litStack.length);
  // Running paren nesting across the whole pass (not just the current
  // `cur` line buffer) -- a forced mid-statement flush (e.g. after a
  // heredoc terminator, or a multiline-literal element break) can leave
  // `cur` holding an unmatched ')' with no '(' of its own, which would
  // make a per-cur-buffer depth count go negative and never reach 0.
  let parenLevel = 0;
  // Whether the current statement's multi-line continuation (string-
  // concat chain or trailing-comma declarator list) bumped the depth
  // (restored at the statement's ';') -- see the continuation check at
  // the top of the loop.
  let inContinuation = false;
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

  // The innermost litStack FRAME as of the token that STARTED the current
  // `cur` line buffer -- flush() derives its mappingContext from the frame
  // that was innermost THEN, not from whatever is on top when the flush
  // finally happens: a mapping element whose value opens its own construct
  // that goes multiline (`"k": funcall(\n  a,\n  b\n) ...`) flushes its
  // header slice while the top frame is the value's 'paren', but the KEY
  // COLON in that slice belongs to the mapping -- judging by the frame at
  // cur-start keeps it tight (and idempotent: the wrapped output
  // re-derives the same decision). A frame REFERENCE, not a stack index:
  // a glued close (`"b": 2 ]);` -- see the close branch) pops the mapping
  // frame while its element is still sitting in `cur`, and the element's
  // colon must still know it was a mapping's. Maintained by pushCur(),
  // the only way tokens enter `cur`.
  // While `cur` holds nothing but CLOSING delimiters (a wrap-trailer-
  // shaped line like `}), "cellw": 4 ])`), keep re-snapshotting: the
  // frame that governs the line's real content (the mapping owning that
  // key colon) is whatever is innermost once the leading closes have
  // popped their frames.
  let curStartTop = undefined;
  let curStartTern = 0;
  let curOnlyCloses = true;
  const pushCur = (tok) => {
    const isClose = tok.text === ')' || tok.text === '}' || tok.text === ']' || tok.text === ':)';
    if (cur.length === 0 || curOnlyCloses) {
      curStartTop = litStack[litStack.length - 1];
      curStartTern = curStartTop ? curStartTop.tern : stmtTern;
      if (cur.length === 0) curOnlyCloses = isClose;
      else if (!isClose) curOnlyCloses = false;
    }
    cur.push(tok);
  };

  // Ternary '?'s awaiting their ':' are tracked PER FRAME (each litStack
  // entry gets a `tern` counter; `stmtTern` covers statement level),
  // mirroring renderLine's per-bracket ternaryStack but across physical
  // lines: a ':' whose '?' sits on an earlier line of the same statement
  // must still be recognized as the ternary's (not tightened as a
  // mapping key colon). ','/';' reset the current level, matching the
  // per-line model.
  let stmtTern = 0;
  // Indent bumps owed by multiline constructs whose CLOSE the source
  // glued onto the last element's line (`..."\n");` -- see the close
  // branch below): the line still renders at the element's depth, and
  // the dedent lands right after it flushes.
  let pendingDedent = 0;
  // One entry per CLOSE token pushed into `cur`, in order: true when that
  // close deferred a bumped frame's dedent (i.e. contributed to
  // pendingDedent). flush() aligns these with `cur`'s close tokens to
  // tell wrapTokens which unmatched closes dedent on a re-parse -- see
  // wrapTokens' `dedentAt`.
  let curCloseBumps = [];
  const pushClose = (tok, deferredBump) => {
    curCloseBumps.push(!!deferredBump);
    pushCur(tok);
  };

  const flush = (extraDedent = 0, mappingContextOverride) => {
    if (cur.length === 0) return;
    const d = Math.max(0, depth - extraDedent);
    // A multi-line mapping literal flushes each "key: value," element as
    // its OWN line/renderLine call, so that call's own (per-call, reset
    // to empty each time) bracketStack never sees the '[' that opened the
    // mapping -- it's on an earlier, already-flushed line. Pass down
    // whether litStack's current top is the enclosing mapping frame so
    // renderLine can still recognize this element's own key:value colon
    // (see wrapTokens/renderLine's `mappingContext` parameter). Callers
    // that flush AFTER already popping the mapping's own litStack frame
    // (the closing-bracket path, which needs `top.kind` to decide other
    // things first) pass the correct answer explicitly via
    // `mappingContextOverride` instead of leaving this to guess wrong.
    let mappingContext;
    if (mappingContextOverride !== undefined) {
      mappingContext = mappingContextOverride;
    } else {
      // Judged by the frame that was innermost when `cur` STARTED (see
      // curStartTop above) -- it may have been popped since, but the
      // frame object still knows what it was.
      mappingContext = !!curStartTop && curStartTop.kind === 'mapping';
    }
    // Which of cur's UNMATCHED closes dedent on a second pass's re-parse
    // (exactly the ones that deferred a bumped frame's dedent above) --
    // keyed by token identity so wrapTokens' recursive slices can consult
    // it regardless of how indexes shift.
    const dedentAt = new Map();
    {
      let bal = 0;
      let qi = 0;
      for (const tok of cur) {
        const c = tok.text;
        if (c === '(' || c === '{' || c === '[') bal++;
        else if (c === ')' || c === '}' || c === ']') {
          const deferred = qi < curCloseBumps.length ? curCloseBumps[qi] : false;
          qi++;
          if (bal > 0) bal--;
          else if (deferred) dedentAt.set(tok, true);
        }
      }
    }
    lines.push(...wrapTokens(cur, d, printWidth, indentUnit, mappingContext, curStartTern, dedentAt));
    cur = [];
    curCloseBumps = [];
    if (pendingDedent > 0) {
      depth = Math.max(0, depth - pendingDedent);
      pendingDedent = 0;
    }
  };

  // Inside a literal, call, or condition that's already gone multiline (or
  // is starting to), a token beginning on a later source line than the
  // previous one preserves that break, matching how the source laid it out
  // instead of collapsing it onto a single line -- e.g. a large mapping/
  // array table, a call whose arguments were one-per-line, a multi-line
  // boolean condition (`if (a &&\n    b)`), or a standalone comment (a
  // `/* --- Section: --- */` header before the next element) that would
  // otherwise glue onto the tail of the PREVIOUS element's line. Called
  // both from the comment-handling branch (which dispatches on `t.kind`
  // before this would otherwise run) and the general per-token check
  // below, so a comment gets the same treatment as any other token.
  // `paren-literal` (the '(' that opens `({`/`([`) is excluded -- it's
  // just a wrapper around the array/mapping frame, which already does
  // this tracking.
  const preserveMultilineBreak = (t) => {
    if (!litStack.length) return;
    const top = litStack[litStack.length - 1];
    const prevInCur = cur[cur.length - 1];
    // Compare against the PREVIOUS token's end line, not its start line --
    // a multi-line string/comment/textblock/template token (one that
    // itself spans several physical lines) must not look like a "break"
    // to whatever immediately follows it on that token's own last line.
    if ((top.kind === 'array' || top.kind === 'mapping' || top.kind === 'paren') && cur.length > 0 &&
        t.line > 0 && prevInCur.line > 0 && t.line > tokenEndLine(prevInCur)) {
      flush();
      // A sanity cap: adversarial/pathological input (a "crasher" test
      // nesting 100+ single-argument calls, one per source line, to probe
      // the driver's own call-depth limit) would otherwise grow the
      // indent by one level per nesting level without bound. Past
      // MAX_MULTILINE_INDENT the line break is still preserved (no
      // collapsing to one giant line) but indentation stops climbing;
      // `bumped` records whether THIS frame actually bumped, so the
      // matching close only dedents for frames that did.
      if (!top.multiline) {
        top.multiline = true;
        if (depth < MAX_MULTILINE_INDENT) { depth++; top.bumped = true; }
      }
    }
  };

  // Blank-line preservation: track source line gaps.
  let lastLine = 0;
  let sawLineZero = false;
  // Pulls trailing comments that START on the same source line as
  // toks[i]'s end into `cur`, returning the new index -- used right
  // before every FORCED flush (a condition close, `else`, a label, a
  // block open...) so `if (cond)  // note` keeps its note at the end of
  // its own rendered line instead of orphaning it onto the body's line.
  const absorbSameLineComments = (i) => {
    while (toks[i + 1] && toks[i + 1].kind === 'comment' &&
           toks[i + 1].line > 0 && toks[i + 1].line === tokenEndLine(toks[i])) {
      pushCur(toks[i + 1]);
      i++;
      lastLine = tokenEndLine(toks[i]);
    }
    return i;
  };
  // Does `nxt` keep a just-closed (possibly one-lined or empty) block's
  // '}' on the current line -- the construct isn't done yet (`} else`, a
  // do-body's `} while`, `};`, `},`) or the block is a call's/functional
  // literal's last argument whose close sticks to it (`});`, `} :)`)?
  const blockStaysOpen = (nxt, isDo) => !!nxt &&
    ((nxt.kind === 'keyword' && (nxt.text === 'else' || (nxt.text === 'while' && isDo))) ||
     (nxt.kind === 'punctuation' && (nxt.text === ';' || nxt.text === ',' || nxt.text === ')')) ||
     (nxt.kind === 'functional' && nxt.text === ':)'));
  for (let idx = 0; idx < toks.length; idx++) {
    const t = toks[idx];

    // Runs first so the blank-line-gap check right below sees the CORRECT
    // (post-flush) `cur` -- otherwise a token that starts a new element of
    // an already-multiline array/mapping/call (still holding the PREVIOUS
    // element unflushed in `cur` at this point) would make the gap check
    // see cur.length > 0 and skip the blank line, silently eating an
    // intentional blank line between two elements or before a standalone
    // section-header comment.
    preserveMultilineBreak(t);

    // A multi-line CONTINUATION at statement level keeps the source's
    // breaks instead of collapsing onto one line, with the continuation
    // lines one level deeper than the statement's first line. Two
    // families, both invisible to the bracket-frame tracking (which
    // already preserves breaks inside a multiline call/literal --
    // preserveMultilineBreak above flushes first there, leaving `cur`
    // empty here):
    //  - STRING-CONCAT CHAINS: adjacent literals (`src = "a\n"\n
    //    "b\n";`, `` `hello`\n  `world`; ``, textblocks), a `+` tail or
    //    a leading `+`, and bare IDENTIFIERS in the chain (`"..." HIY\n
    //    "..."` -- a string-valued macro juxtaposed between literals, the
    //    color-macro idiom). A chain STARTS only at a real literal
    //    boundary (a template's own INTERNAL interpolation tokens never
    //    trigger); once started, any break between chain links
    //    (pieces / identifiers / '+') is preserved until the ';'.
    //  - A TRAILING COMMA: a declarator/element list with no bracket
    //    frame (`int a, b,\n  c;`) keeps its break after the comma.
    if (cur.length > 0 && t.line > 0) {
      const prevInCur = cur[cur.length - 1];
      const tPiece = t.kind === 'string' || t.kind === 'textblock' ||
        (t.kind === 'template' && t.text.startsWith('`'));
      let continues;
      if (prevInCur.text === ',') {
        continues = true;
      } else if (inContinuation) {
        continues = isChainLink(prevInCur) && (tPiece || t.kind === 'identifier' || t.text === '+');
      } else {
        continues =
          (tPiece && (endsStringPiece(prevInCur) || prevInCur.kind === 'identifier' ||
                      (prevInCur.text === '+' && cur.length >= 2 && endsStringPiece(cur[cur.length - 2])))) ||
          ((t.kind === 'identifier' || t.text === '+') && endsStringPiece(prevInCur));
      }
      if (continues && prevInCur.line > 0 && t.line > tokenEndLine(prevInCur)) {
        flush();
        if (!inContinuation) { inContinuation = true; depth++; }
      }
    }

    // Blank lines follow the source EXACTLY -- a two-blank separator
    // between sections stays two blanks, not a canonicalized one (the
    // trailing-blank trim at EOF below is the only exception).
    // Tokens inside a template interpolation carry line 0; after such a
    // run, the next real-line token must RESYNC lastLine instead of
    // comparing against the pre-template value -- a forced flush inside
    // `${...}` (a `//` comment, a directive) otherwise manufactures a
    // blank-line gap that GROWS by one every pass.
    if (t.line > 0 && sawLineZero) {
      lastLine = t.line;
      sawLineZero = false;
    }
    // A BLANK line at statement level is a hard separator even when no
    // ';' intervened -- a bare top-level macro invocation (`LONG` with
    // the whole function body in the #define, marco_crlf.lpc) must not
    // glue onto the next declaration and eat the gap. Inside brackets
    // the frame machinery already handled the break above.
    if (cur.length > 0 && t.line > 0 && lastLine > 0 && t.line - lastLine > 1 &&
        parenLevel === 0 &&
        (litStack.length === 0 || litStack[litStack.length - 1].kind === 'block')) {
      flush();
    }
    if (t.line > 0 && lastLine > 0 && t.line - lastLine > 1 && cur.length === 0
        && lines.length > 0 && lines[lines.length - 1] !== '') {
      for (let b = t.line - lastLine - 1; b > 0; b--) lines.push('');
    }
    if (t.line > 0) lastLine = t.line + (t.text.match(/\n/g) || []).length;
    else sawLineZero = true;

    if (t.kind === 'directive') {
      flush();
      lines.push(t.text.replace(/[ \t]+(\r?)$/g, '$1'));
      continue;
    }
    if (t.kind === 'comment') {
      // (preserveMultilineBreak already ran for `t` at the top of the loop
      // -- a comment starting a new source line inside an already-
      // multiline array/mapping/call, e.g. a "/* --- Section: --- */"
      // header before the next element, was flushed there instead of
      // gluing onto the tail of the PREVIOUS element's `cur`.)
      // standalone comment gets its own line; trailing comment joins.
      // A '//' comment runs to end of physical line, so nothing may
      // follow it on the same rendered line -- force a flush, or a
      // second format pass would swallow the next tokens into it.
      if (cur.length === 0) {
        // A LEADING comment whose next token starts on the comment's own
        // (end) line stays glued to it (`/*x*/ 2_000_000,` as one wrapped
        // mapping/array element) -- splitting them made a printWidth-
        // wrapped literal non-idempotent: pass 1's wrapTokens kept them
        // together, pass 2's main loop split them apart.
        const rawNxt = toks[idx + 1];
        if (rawNxt && rawNxt.line > 0 && rawNxt.line === tokenEndLine(t)) {
          pushCur(t);
        } else {
          lines.push(indentUnit.repeat(depth) + t.text);
        }
      } else {
        pushCur(t);
        if (t.text.startsWith('//')) flush();
      }
      continue;
    }
    if (t.kind === 'textblock') {
      // A heredoc's terminator word ends the token mid-line (e.g. at
      // "TEXT" in "TEXT;" -- the driver rescans anything after it on
      // that line as ordinary code, see parseHeredoc() in
      // lexer_utils.cc). Whatever the source wrote there stays there:
      // `TEXT;` keeps its ';' glued to the terminator (the pristine
      // corpus spelling), `ENDHELP);` keeps the call close -- no forced
      // break. The token just joins `cur` like any other and the normal
      // statement machinery decides the line ending.
      pushCur(t);
      continue;
    }

    // A `{ ... }` block that the source already wrote on one physical
    // line stays one line, whatever its statement count, as long as the
    // rendered result still fits printWidth (see tryOneLinerBlock) --
    // otherwise it falls through to the normal expansion below.
    // Idempotent: a second pass sees this exact single-line span again
    // and re-derives the same decision from it.
    if (t.text === '{') {
      const prevTok = prevNonComment(toks, idx);
      const isLiteral = !!prevTok && prevTok.text === '(';
      // For the two fast paths below: a following `while` keeps the block
      // open on the line only when this really is a `do` body (same
      // reasoning as the generic block-close path's staysOpen check).
      const isDoBody = !!prevTok && prevTok.kind === 'keyword' && prevTok.text === 'do';
      if (!isLiteral) {
        // A genuinely empty block -- nothing at all between '{' and '}',
        // not even a comment -- always collapses to '{}' regardless of
        // source layout or printWidth: there's no content whose line-break
        // choice could matter. (A comment inside falls through to the
        // normal one-liner/expansion path below instead of being silently
        // dropped or corrupting into `{ // ... }`.)
        if (toks[idx + 1] && toks[idx + 1].text === '}') {
          pushCur(t);
          pushClose(toks[idx + 1], false);
          // Jumping `idx` past a span skips the blank-line-gap tracking at
          // the top of the loop for every token in between -- update it
          // here to the span's actual last line, or the NEXT token (which
          // may be many source lines later, e.g. the outer block's own
          // close) would look like a spurious blank-line gap.
          lastLine = tokenEndLine(toks[idx + 1]);
          if (parenLevel === 0) {
            if (!blockStaysOpen(toks[idx + 2], isDoBody)) {
              idx = absorbSameLineComments(idx + 1) - 1;
              flush();
              // This (possibly empty) block may itself be the body a
              // dangling if/while/for/foreach/else was waiting on (e.g.
              // `if (a)\n  if (b) {}`) -- resolve it now that it's
              // actually rendered, same as the generic block-close path
              // below does for a multi-statement block. Neither fast path
              // here pushes/pops a litStack frame for this '{'/'}' pair,
              // so `litStack.length` is unchanged and still matches
              // whatever snapshot the dangling push recorded.
              drainDanglingBody();
            }
          }
          idx += 1;
          continue;
        }
        const oneLiner = tryOneLinerBlock(toks, idx, cur, depth, printWidth, indentUnit);
        if (oneLiner) {
          // Closes route through pushClose (deferring nothing) so flush's
          // curCloseBumps queue stays aligned with cur's close tokens --
          // a bare pushCur here shifted every later entry, handing a
          // GLUED close's deferred dedent to the wrong token and breaking
          // narrow-printWidth wrap idempotency.
          for (let k = idx; k <= oneLiner.closeIdx; k++) {
            const tk = toks[k];
            if (tk.text === ')' || tk.text === '}' || tk.text === ']') pushClose(tk, false);
            else pushCur(tk);
          }
          // parenLevel > 0 means this one-liner is nested inside an
          // enclosing expression (e.g. a function-literal argument to a
          // call) -- whatever follows it (')', ',', an operator...) is
          // for the ENCLOSING construct to decide when to flush, same as
          // any other token addition. Only at parenLevel 0 (this block
          // sits directly in statement position, like an if/while body)
          // do we apply the control-flow "stays open" check.
          if (parenLevel === 0) {
            if (!blockStaysOpen(toks[oneLiner.closeIdx + 1], isDoBody)) {
              oneLiner.closeIdx = absorbSameLineComments(oneLiner.closeIdx);
              flush();
              // This one-liner block may itself be the body a dangling
              // if/while/for/foreach/else was waiting on -- resolve it now
              // that it's actually rendered (not e.g. an `if {...} else`
              // chain still gluing more onto the same `cur` line, where
              // the chain -- and hence the dangling body it may still
              // constitute -- isn't done yet).
              drainDanglingBody();
            }
          }
          idx = oneLiner.closeIdx;
          continue;
        }
      }
    }

    if (t.text === '}' || t.text === ']' || t.text === ')') {
      const top = litStack.pop();
      pruneDanglingBody();
      if (t.text === ')') parenLevel = Math.max(0, parenLevel - 1);
      if (top && (top.kind === 'array' || top.kind === 'mapping' || top.kind === 'paren')) {
        if (top.multiline) {
          // Follow the source's own choice for the CLOSE, same as every
          // other line-break decision: if the close began a new source
          // line, preserveMultilineBreak (top of the loop) already
          // flushed and `cur` is empty here -- the close gets its own
          // dedented line. A close written on the SAME line as the last
          // element (`..."\n");`, `2 ])`) stays glued to it instead of
          // being forced onto its own line; the frame's indent bump then
          // lasts until that line actually flushes (the element renders
          // at element depth), applied via flush()'s pendingDedent.
          if (cur.length > 0) {
            if (top.bumped) pendingDedent++;
          } else if (top.bumped) {
            depth = Math.max(0, depth - 1);
          }
        }
        pushClose(t, top.multiline && top.bumped && cur.length > 0);
        // A brace-less if/while/for/foreach body that the source wrote on
        // its own line (as opposed to `if (x) stmt();`, which stays
        // compact) preserves that break instead of being glued onto the
        // condition -- see the ';'/block-close handling below for where
        // the matching depth bump is undone.
        if (top.controlFlow) {
          const nxt = nextNonComment(toks, idx);
          if (nxt && nxt.text !== '{' && t.line > 0 && nxt.line > 0 && nxt.line > tokenEndLine(t)) {
            idx = absorbSameLineComments(idx);
            flush();
            depth++;
            danglingBodyStack.push({ lit: litStack.length, kw: top.controlFlow });
          }
        }
        continue;
      }
      if (top && (top.kind === 'index' || top.kind === 'paren-literal')) {
        pushClose(t, false);
        continue;
      }
      // block close ('}' -- '[' never opens a block, and ')' is handled
      // above). If the last case/
      // default label in this block left its body-indent bump active
      // (no further label came along to undo it -- see the ':' handling
      // below), undo it now so the closing brace lines back up with the
      // block it closes, not with the case body.
      flush();  // pending content glued before this '}' still renders at
                // the case BODY depth -- dedent only after it's out
      if (top && top.caseBodyOpen) depth = Math.max(0, depth - 1);
      depth = Math.max(0, depth - 1);
      pushClose(t, false);
      // '}' followed by else/while(do)/';'/',' stays open on the line --
      // the construct (if/else chain, do-while) isn't done yet, so a
      // dangling if/while/for/foreach/else this block might itself be the
      // (brace-full) body of, e.g. `if (a)\n  if (b) {\n    s();\n  } else
      // {\n    t();\n  }`, must NOT resolve yet either: it stays pending
      // until the chain's true end (mirrors why `flush(0)` below is
      // withheld in exactly the same cases).
      if (!blockStaysOpen(toks[idx + 1], !!top && top.isDo)) {
        idx = absorbSameLineComments(idx);
        flush(0);
        drainDanglingBody();
      }
      continue;
    }

    // (preserveMultilineBreak already ran for `t` at the top of the loop.)
    pushCur(t);
    if (t.text === '(') {
      parenLevel++;
      const nxt = nextNonComment(toks, idx);
      const isLiteralOpener = !!nxt && (nxt.text === '{' || nxt.text === '[');
      // Tags the '(' of an if/while/for/foreach condition with WHICH
      // keyword it belongs to -- once its matching ')' closes (below), a
      // brace-less body that starts on a new source line gets its own
      // line-break preserved instead of being glued onto the condition
      // (switch/do always take a real block so tagging them would never
      // fire; not worth excluding). The keyword is carried into the
      // dangling-body entry so a later `else` can resolve down to exactly
      // the `if` it binds to.
      const prevForControlFlow = prevNonComment(toks, idx);
      const isControlFlow = !!prevForControlFlow && prevForControlFlow.kind === 'keyword' &&
        (prevForControlFlow.text === 'if' || prevForControlFlow.text === 'while' ||
         prevForControlFlow.text === 'for' || prevForControlFlow.text === 'foreach');
      litStack.push({ kind: isLiteralOpener ? 'paren-literal' : 'paren', multiline: false, bumped: false,
                       tern: 0, controlFlow: isControlFlow ? prevForControlFlow.text : false });
    }

    if (t.text === '{' || t.text === '[') {
      const prev = prevNonComment(toks, idx);
      const isLiteral = !!prev && prev.text === '(';
      if (t.text === '{') {
        litStack.push({ kind: isLiteral ? 'array' : 'block', multiline: false, caseBodyOpen: false, bumped: false,
                        tern: 0, isDo: !isLiteral && !!prev && prev.kind === 'keyword' && prev.text === 'do' });
        if (isLiteral) continue;
        idx = absorbSameLineComments(idx);
        flush();  // the '{' line itself sits at the CURRENT depth --
                  // depth only increments for what comes after it
        depth++;
        continue;
      }
      litStack.push({ kind: isLiteral ? 'mapping' : 'index', multiline: false, bumped: false, tern: 0 });
      continue;
    }
    // `else` has no parens of its own to hang the if/while/for/foreach
    // dangling-body check on (above) -- if it's neither `else if (...)`
    // (the nested if's OWN condition-close handles it) nor `else {...}`
    // (the normal block-open path handles it), and its bare body starts
    // on a new source line, preserve that break the same way.
    if (t.kind === 'keyword' && t.text === 'else') {
      const nxt = nextNonComment(toks, idx);
      if (nxt && nxt.text !== '{' && nxt.text !== 'if' &&
          t.line > 0 && nxt.line > 0 && nxt.line > tokenEndLine(t)) {
        idx = absorbSameLineComments(idx);
        flush();
        depth++;
        danglingBodyStack.push({ lit: litStack.length, kw: 'else' });
      }
    }
    // Cross-line ternary bookkeeping (see stmtTern above). Label colons
    // never reach here with a pending '?' at their own level, and a
    // mapping key colon only decrements when a ternary is actually open
    // -- same order renderLine uses.
    if (t.text === '?') {
      if (litStack.length) litStack[litStack.length - 1].tern++;
      else stmtTern++;
    } else if (t.text === ',' || t.text === ';') {
      if (litStack.length) litStack[litStack.length - 1].tern = 0;
      else stmtTern = 0;
    } else if (t.text === ':') {
      if (litStack.length) {
        const f = litStack[litStack.length - 1];
        if (f.tern > 0) f.tern--;
      } else if (stmtTern > 0) stmtTern--;
    }
    // A ';' ends a statement when it's at the top level OR directly inside
    // a real `{...}` block, even one nested in parens (an anonymous
    // `function() { ... }` argument to call_out/map/filter) -- the old
    // `parenLevel === 0`-only gate glued that function body's statements
    // onto one line AND never drained its dangling-body entries, leaking
    // one indent level per occurrence for the rest of the file. A ';'
    // whose innermost frame is NOT a block (a `for (;;)` header's, inside
    // the for's own 'paren' frame) still stays merged, as before.
    if (t.text === ';' &&
        (parenLevel === 0 || (litStack.length > 0 && litStack[litStack.length - 1].kind === 'block'))) {
      // A same-source-line trailing comment (`private int x = 1; // note`)
      // must not be severed onto its own line -- defer the flush decision
      // to the comment-handling branch above, which already knows how to
      // merge a trailing `//` comment onto the statement's line and THEN
      // flush (or leave a trailing `/* */` in cur for more code to follow).
      const rawNxt = toks[idx + 1];
      if (rawNxt && rawNxt.kind === 'comment' && rawNxt.line === t.line) continue;
      // Statement groups the source wrote on ONE physical line stay
      // merged -- not just on `case` lines (the aligned lookup-table
      // idiom) but anywhere (`a = 0; a ||= 9; ASSERT_EQ(9, a);` as one
      // aligned check line). The group ends where the source's line
      // does, or at a label/block-close, which always start fresh.
      {
        const nxt = nextNonComment(toks, idx);
        const staysMerged = !!nxt && nxt.line > 0 && t.line > 0 && nxt.line === t.line &&
          !(nxt.kind === 'keyword' && (nxt.text === 'case' || nxt.text === 'default')) &&
          !(nxt.kind === 'punctuation' && nxt.text === '}');
        if (staysMerged) continue;
      }
      flush();
      // (after the flush: the statement's LAST line still renders at the
      // continuation depth)
      if (inContinuation) {
        inContinuation = false;
        depth = Math.max(0, depth - 1);
      }
      // A brace-less if/while/for/foreach/else body that turned out to be
      // a plain statement resolves here, at its own terminating ';' --
      // see the `danglingBodyStack` declaration above for why a chain
      // with no real block in between resolves all at once. EXCEPT when
      // an `else` follows: it binds to the NEAREST unresolved `if`, so
      // only entries above-and-including that `if` resolve now -- the
      // `else` then renders aligned with its `if` (`if (a)\n  if (b)\n
      // c();\n  else\n    d();` keeps the else under the inner if,
      // matching the actual binding) and pushes its own entry when its
      // bare body follows on a new line.
      const afterSemi = nextNonComment(toks, idx);
      if (afterSemi && afterSemi.kind === 'keyword' && afterSemi.text === 'else' &&
          danglingBodyStack.some((e) => e.lit === litStack.length && e.kw === 'if')) {
        while (danglingBodyStack.length &&
               danglingBodyStack[danglingBodyStack.length - 1].lit === litStack.length) {
          const e = danglingBodyStack.pop();
          depth = Math.max(0, depth - 1);
          if (e.kw === 'if') break;
        }
      } else {
        drainDanglingBody();
      }
      continue;
    }
    // Gated on "directly inside a block" (litStack top) and parenLevel:
    // only a colon there can be the label's own colon. Without the gate, a
    // MAPPING key colon on a merged case line whose value continues on the
    // next source line (`case 1: m = ([ "a":\n  2 ]); break;`) was treated
    // as a label colon -- flushing mid-expression and stamping
    // `caseBodyOpen`/`depth++` onto the MAPPING's frame, which the
    // array/mapping close path never undoes: a permanent +1 indent drift
    // for the rest of the file.
    if (t.text === ':' && cur.length >= 2 &&
        (cur[0].text === 'case' || cur[0].text === 'default') &&
        parenLevel === 0 &&
        (litStack.length === 0 || litStack[litStack.length - 1].kind === 'block')) {
      // A source-line-preserving carve-out, same philosophy as the
      // array/mapping multiline tracking above: `case X:` followed by its
      // body on the SAME source line (the common aligned lookup-table
      // idiom, e.g. `case '"':  esc = '"';  break;`) stays one rendered
      // line; a label written on its own line (including stacked
      // fallthrough labels) keeps its own line. Idempotent because a
      // second pass sees exactly the line grouping this pass emitted.
      //
      // A non-merged label's body renders one level deeper than the
      // label (matching the corpus's overwhelmingly common style) --
      // tracked as a bump on the switch's own block frame (the innermost
      // litStack 'block' entry) so stacked fallthrough labels undo the
      // PREVIOUS label's bump before lining back up with it, and the
      // switch's closing '}' (above) undoes a still-open last label's
      // bump. A merged (same-line) label+body needs no bump: it's one
      // line, nothing renders "under" it.
      const nxt = nextNonComment(toks, idx);
      const merged = !!nxt && nxt.line > 0 && t.line > 0 && nxt.line === t.line;
      const frame = litStack[litStack.length - 1];
      if (frame && frame.caseBodyOpen) {
        depth = Math.max(0, depth - 1);
        frame.caseBodyOpen = false;
      }
      if (!merged) {
        idx = absorbSameLineComments(idx);
        flush(0);
        if (frame) { frame.caseBodyOpen = true; depth++; }
      }
      continue;
    }
    // A modifier-section label (`public:`, `nomask private:` -- every
    // token before the ':' is a modifier, at statement level) follows the
    // source's line choice, same as a case label: written on its own line
    // it keeps that line (the following declarations are separate
    // statements, not the label's continuation); written inline
    // (`public: void f() {}`) it stays merged. No depth bump -- section
    // labels sit at the same level as the declarations they govern.
    if (t.text === ':' && parenLevel === 0 && cur.length >= 2 &&
        cur.slice(0, -1).every((c) => c.kind === 'modifier')) {
      const nxt = nextNonComment(toks, idx);
      if (nxt && nxt.line > 0 && t.line > 0 && nxt.line > tokenEndLine(t)) {
        idx = absorbSameLineComments(idx);
        flush();
      }
      continue;
    }
  }
  flush();
  while (lines.length && lines[lines.length - 1] === '') lines.pop();
  // A line entry can itself embed raw newlines (an unterminated string/
  // comment/text block token swallows to EOF, trailing newline included)
  // -- don't add a second one, or each re-format grows another blank line.
  let joined = lines.join('\n');
  // An empty or whitespace-only source has nothing to render -- lines ends
  // up empty, and the trailing-newline guarantee below must not manufacture
  // content (`formatLPC("")` must stay `""`, not become `"\n"`).
  if (joined === '') return '';
  if (!joined.endsWith('\n')) joined += '\n';
  // A CRLF source stays CRLF (testsuite/single/tests/compiler/
  // at_block_crlf.lpc deliberately exercises the driver's CRLF handling
  // and is kept dos-style by its directory's .gitattributes) -- decided
  // by the source's dominant newline flavor. Token texts spanning lines
  // (textblocks, comments) already carry the source's own CRLFs;
  // normalize first so nothing double-converts.
  const crlfCount = (source.match(/\r\n/g) || []).length;
  const bareLfCount = (source.match(/\n/g) || []).length - crlfCount;
  // ONLY when the source is uniformly CRLF -- the conversion rewrites
  // every newline in the output, including those INSIDE multi-line
  // string/template/heredoc tokens, so a mixed-EOL file (a CRLF file
  // whose heredoc deliberately contains bare-LF lines) must be left
  // alone or the token's VALUE would change.
  if (crlfCount > 0 && bareLfCount === 0) {
    joined = joined.replace(/\r\n/g, '\n').replace(/\n/g, '\r\n');
  }
  return joined;
}

// A token's line + however many newlines its own text contains -- the
// physical source line its LAST character sits on (strings/comments/
// textblocks/templates can span several lines by themselves).
function tokenEndLine(t) {
  if (t.line <= 0) return t.line;
  return t.line + (t.text.match(/\n/g) || []).length;
}

// Does this token END a string-like literal piece -- a string, a
// textblock, or a template's closing token (a whole `` `x` `` or a
// closing `}b` `` segment; template segments only lex correctly in
// sequence, but the LAST one always ends with the closing backtick)?
// Used by the statement-level continuation check in formatLPC.
function endsStringPiece(tok) {
  return !!tok && (tok.kind === 'string' || tok.kind === 'textblock' ||
                   (tok.kind === 'template' && tok.text.endsWith('`')));
}

// A token that can sit between the literal pieces of a statement-level
// string-concat chain: another piece, a bare identifier (a string-valued
// macro), or the '+' operator.
function isChainLink(tok) {
  return !!tok && (endsStringPiece(tok) || tok.kind === 'identifier' || tok.text === '+');
}

function prevNonComment(toks, idx) {
  for (let k = idx - 1; k >= 0; k--) {
    if (toks[k].kind !== 'comment') return toks[k];
  }
  return null;
}

function nextNonComment(toks, idx) {
  for (let k = idx + 1; k < toks.length; k++) {
    if (toks[k].kind !== 'comment') return toks[k];
  }
  return null;
}

// Mirrors the driver's own preprocessing of a directive's text before any
// macro analysis (fold_backslash_newlines + strip_directive_comments in
// lexer_rules_pp.cc): `\`-line-splices vanish, block comments become a
// single space (but NOT inside string/char literals -- the driver skips
// quoted spans atomically), and a `//` comment ends the body. The
// stringize analysis below must run on this folded form or a `#define
// SC(x) #/*c*/x`, a `# \<newline>x`, or a spliced parameter list all
// hide their stringize from the detector -- driver-verified semantic
// corruption (the call-site argument then gets re-spaced).
function foldDirectiveText(text) {
  let out = '';
  let i = 0;
  const n = text.length;
  let quote = null;
  while (i < n) {
    const c = text[i];
    if (c === '\\') {
      if (text[i + 1] === '\n') { i += 2; continue; }
      if (text[i + 1] === '\r' && text[i + 2] === '\n') { i += 3; continue; }
      out += c;
      if (i + 1 < n) out += text[i + 1];
      i += 2;
      continue;
    }
    if (quote) {
      if (c === quote) quote = null;
      out += c;
      i++;
      continue;
    }
    if (c === '"' || c === "'") { quote = c; out += c; i++; continue; }
    if (c === '/' && text[i + 1] === '*') {
      const end = text.indexOf('*/', i + 2);
      out += ' ';
      i = end === -1 ? n : end + 2;
      continue;
    }
    if (c === '/' && text[i + 1] === '/') break;
    out += c;
    i++;
  }
  return out;
}

// Detects `#define NAME(params) body` directives whose body stringizes one
// of its parameters via '#param' ('##' alone is token paste and doesn't
// care about argument spelling). '#x' captures the parameter's EXACT
// SOURCE SPELLING as written at the call site (docs/lpc/preprocessor/) --
// re-spacing operators in that argument (`1+2` -> `1 + 2`) silently
// changes what it stringizes to, so those call-site arguments must never
// be touched by the normal formatting pass (see maskStringizeArguments).
//
// Returns a Map of name -> per-parameter stringize flags, OR-ing together
// EVERY definition of that name anywhere in the file. Deliberately NOT
// position- or #if-aware: the formatter can't evaluate preprocessor
// conditions, and a `#define STR(x) x` redefinition sitting inside an
// `#if 0` (which the driver never executes) must not strip protection
// from later call sites. Over-masking is the safe direction -- a masked
// argument merely keeps its original spacing; under-masking silently
// changes what the program stringizes (driver-verified).
//
// Limitation: this only sees #define directives written in THIS file -- a
// stringize macro defined only in an #include'd header is invisible here
// (no corpus file currently does this; seeing it would require following
// #include paths, which this tokenizer-driven formatter deliberately
// doesn't attempt).
function collectStringizeMacros(toks) {
  const macros = new Map();
  for (const t of toks) {
    if (t.kind !== 'directive') continue;
    const text = foldDirectiveText(t.text);
    // No space allowed between NAME and '(' -- same as C, that space is
    // what distinguishes a function-like macro from an object-like one
    // whose body happens to start with a parenthesized expression.
    const m = /^#\s*define\s+([A-Za-z_]\w*)\(([^)]*)\)([\s\S]*)$/.exec(text);
    if (!m) continue;
    const [, name, paramsRaw, body] = m;
    const params = paramsRaw.split(',').map((p) => p.trim()).filter(Boolean);
    const flags = params.map((p) => {
      if (!/^[A-Za-z_]\w*$/.test(p)) return false; // skips a literal '...'
      // The driver's substitute() consumes '#' PAIRS as token-paste first;
      // a leftover odd '#' stringizes. So `#x` and `###x` stringize,
      // `##x` and `####x` paste -- an ODD-length '#' run means stringize.
      const re = new RegExp('#+\\s*' + p + '\\b', 'g');
      let hit;
      while ((hit = re.exec(body)) !== null) {
        if (/^#+/.exec(hit[0])[0].length % 2 === 1) return true;
      }
      return false;
    });
    const existing = macros.get(name);
    if (existing) {
      for (let k = 0; k < Math.max(existing.length, flags.length); k++) {
        existing[k] = !!existing[k] || !!flags[k];
      }
    } else {
      macros.set(name, flags);
    }
  }
  return macros;
}

// For every call site of a macro tracked in `macros`, freezes each
// STRINGIZED argument's token span into a single 'verbatim' token
// carrying that span's exact original source text (via absolute
// offsets) -- run BEFORE the main formatting pass so nothing downstream
// ever re-spaces it (the rest of format.mjs treats an unrecognized token
// kind as an opaque atom, same as it already does for
// 'string'/'comment'/'textblock'). A macro call nested inside a
// stringized argument (`STR(STR(1+2))`) is naturally covered too: the
// OUTER argument's span already contains the inner call's tokens,
// matching real preprocessor semantics where '#'/'##' operate on the
// argument's raw, unexpanded spelling. Non-stringized positions/
// arguments of the same call are left as ordinary tokens.
function maskStringizeArguments(toks, source, macros) {
  if (macros.size === 0) return toks;
  const spans = []; // {start, end}: inclusive token indices to freeze
  for (let i = 0; i < toks.length; i++) {
    const t = toks[i];
    // A macro's name can shadow a reserved word -- the driver resolves
    // macros BEFORE reserved-word lookup (lpc_lex_resolve_identifier), so
    // `#define string(x) #x` + `string(5+6)` expands and stringizes even
    // though our tokenizer calls that call-site token a 'type'. Any
    // word-shaped token can be a macro call.
    if (!/^[A-Za-z_]\w*$/.test(t.text)) continue;
    const nxt = toks[i + 1];
    if (!nxt || nxt.text !== '(') continue;
    const flags = macros.get(t.text);
    if (!flags || !flags.some(Boolean)) continue;
    // Argument splitting must mirror the DRIVER's macro-argument
    // collector, which nests only the '(' CHARACTER (and skips quoted
    // spans -- single tokens here): a comma inside `x[1+2,y+3]` REALLY
    // splits macro arguments in the driver, so it must split here too or
    // the per-argument stringize flags land on the wrong spans
    // (driver-verified). The functional-literal tokens '(:'/':)' contain
    // a raw '('/')' character, so the driver's char-level count nests
    // them -- count them here too.
    let depth = 0;
    let close = -1;
    const commas = [];
    for (let j = i + 1; j < toks.length; j++) {
      const c = toks[j].text;
      if (c === '(' || c === '(:') depth++;
      else if (c === ')' || c === ':)') {
        depth--;
        if (depth === 0) { close = j; break; }
      } else if (c === ',' && depth === 1) {
        commas.push(j);
      }
    }
    if (close < 0) continue;
    let argStart = i + 2;
    const boundaries = [...commas, close];
    const callSpans = [];
    for (let k = 0; k < boundaries.length; k++) {
      const argEnd = boundaries[k] - 1;
      if (argEnd >= argStart && flags[k]) callSpans.push({ start: argStart, end: argEnd });
      argStart = boundaries[k] + 1;
    }
    // Because the driver splits arguments at commas inside '['/'{' too, a
    // stringized span can be bracket-UNBALANCED (`FB(x[1+2,y+3], 9)` -- the
    // '#b' span is `y+3]`, whose ']' closes a '[' opened in the previous
    // argument). Swallowing an unmatched bracket into an opaque verbatim
    // token desyncs the main loop's litStack (its '[' frame never sees its
    // close), so in that case freeze the WHOLE call instead -- name through
    // ')', always balanced. Over-masking only preserves the other
    // arguments' original spacing, which is the safe direction.
    if (callSpans.every((s) => spanIsBalanced(toks, s.start, s.end))) {
      spans.push(...callSpans);
    } else {
      spans.push({ start: i, end: close });
    }
  }
  if (spans.length === 0) return toks;
  spans.sort((a, b) => a.start - b.start);
  const out = [];
  let si = 0;
  for (let i = 0; i < toks.length;) {
    // Discard any span already swallowed by a bigger one we just froze
    // (the nested-call case above) before checking the current position.
    while (si < spans.length && spans[si].start < i) si++;
    if (si < spans.length && spans[si].start === i) {
      const { start, end } = spans[si];
      const first = toks[start];
      const last = toks[end];
      out.push({
        kind: 'verbatim', text: source.slice(first.start, last.end),
        line: first.line, col: first.col, start: first.start, end: last.end,
      });
      i = end + 1;
      si++;
      continue;
    }
    out.push(toks[i]);
    i++;
  }
  return out;
}

// The full nesting model shared by every depth walker that must treat a
// functional literal as nested (its '(:'/':)' delimiters contain raw
// parens): spanIsBalanced, directCommas, pendingTernaryCount.
// (matchingBraceEnd and the main loop's litStack deliberately track only
// the three real bracket pairs; maskStringizeArguments' argument scanner
// deliberately nests only '('/'(:' -- the driver's own collector model.)
const NEST_OPEN = new Set(['(', '{', '[', '(:']);
const NEST_CLOSE = new Set([')', '}', ']', ':)']);

// Do the bracket tokens within toks[start..end] (inclusive) all pair up
// internally? Used by maskStringizeArguments to decide whether a span is
// safe to freeze as one opaque token.
function spanIsBalanced(toks, start, end) {
  let d = 0;
  for (let j = start; j <= end; j++) {
    const c = toks[j].text;
    if (NEST_OPEN.has(c)) d++;
    else if (NEST_CLOSE.has(c)) {
      d--;
      if (d < 0) return false;
    }
  }
  return d === 0;
}

// Index of the bracket matching toks[openIdx] (any of '(', '{', '['),
// scanning forward and counting ALL bracket types together (so a call or
// literal nested inside is skipped as a unit). -1 if unmatched, or if a
// stray close is seen before the matching one (malformed input -- treat
// as "can't tell", never misassociate).
function matchingBraceEnd(toks, openIdx) {
  let depth = 0;
  for (let i = openIdx; i < toks.length; i++) {
    const c = toks[i].text;
    if (c === '(' || c === '{' || c === '[') depth++;
    else if (c === ')' || c === '}' || c === ']') {
      depth--;
      if (depth === 0) return i;
      if (depth < 0) return -1;
    }
  }
  return -1;
}

// Is the block opened by toks[openIdx] eligible to stay a single rendered
// line: matched, written on one source line already (open/close share a
// line -- any internal break would push the close to a later line, so
// this IS the source's own choice, whatever the statement count:
// `void event_ping(object from, int v) { origin = from; value = v + 1; }`
// stays exactly as written, nested one-line blocks included), and the
// whole candidate line (whatever's already in `cur` plus this block)
// fits printWidth.
function tryOneLinerBlock(toks, openIdx, cur, depth, printWidth, indentUnit) {
  const closeIdx = matchingBraceEnd(toks, openIdx);
  if (closeIdx < 0) return null;
  const openTok = toks[openIdx];
  const closeTok = toks[closeIdx];
  if (openTok.line <= 0 || closeTok.line <= 0 || openTok.line !== closeTok.line) return null;
  const prevInCur = cur[cur.length - 1];
  if (prevInCur && prevInCur.line > 0 && prevInCur.line !== openTok.line) return null;
  const candidate = [...cur, ...toks.slice(openIdx, closeIdx + 1)];
  const rendered = indentUnit.repeat(depth) + renderLine(candidate);
  if (rendered.length > printWidth) return null;
  return { closeIdx };
}

// '[' opens indexing (`a[0]`) or, right after '(', a mapping literal
// (`([...])`) -- neither ever wants a space before the '[' ITSELF (that's
// a separate question from whether there's a space just inside it, see
// bracketStack below). '..' is the range operator (`a[0..2]`) and stays
// tight on both sides; '...' hugs a spread expression (`extra...`) but
// keeps its space as the bare varargs marker (`f(int a, ...)`) -- see
// the endsExpression-gated rule in renderLine. `(:`/`:)` (functional literal delimiters) are
// NOT tight -- corpus convention is a space on the inner side
// (`(: $1 + $2 :)`), confirmed against the pristine corpus (spaced
// outnumbers tight ~615:1 at token level).
// '{'/'}' and '['/']' immediately inside their OWN pair are handled
// separately (below, via bracketStack): a NON-empty array/mapping
// literal (`({ 1, 2, 3 })`, `([ "a": 1 ])`) gets a padding space just
// inside the brackets so its content is never flush against the
// delimiter -- but an EMPTY one (`({})`, `([])`, matching the pristine
// corpus's dominant convention), indexing (`a[0]`, never empty), and a
// one-line block's OWN close (`{}` when genuinely empty, i.e. no
// statement at all) all stay tight.
// '++'/'--' are deliberately NOT here: tight-before is only correct in
// POSTFIX position (`i++`, `a[0]--`), decided positionally in renderLine
// via endsExpression(prev) -- a PREFIX `++count` after a keyword or
// operator keeps its space (`return ++count;`, `x = ++i;`), it must not
// glue into `return++count`.
const NO_SPACE_BEFORE = new Set([';', ',', ')', '.', '->', '?.', '.?', '[', '..']);
const NO_SPACE_AFTER = new Set(['(', '.', '::', '->', '?.', '.?', '!', '~', '..', '$']);

// '::' is tight on BOTH sides only in its qualified form (`efun::foo()`,
// `master::foo()`, `some_inherit_label::foo()`) -- confirmed against the
// pristine (pre-formatting) corpus: every `identifier::`/`efunkw::` site
// is tight. Used bare, with no left-hand qualifier (the "call the nearest
// inherited/efun version directly" form), it keeps normal spacing before
// it, same as any other token starting a new expression -- `return
// ::render()`, `ASSERT_EQ(42, ::proto_fn())` are how the corpus actually
// writes it, not `return::render()`. (NO_SPACE_AFTER above still always
// applies -- `::` is tight against what follows it in both forms.)
function precedesQualifiedScope(tok) {
  return !!tok && (tok.kind === 'identifier' || tok.kind === 'efunkw' || tok.kind === 'type');
}

// Keywords used like a function call (`new(...)`, `catch(...)`) rather
// than a control-flow construct (`if (...)`, `while (...)`) -- these get
// no space before '('. Confirmed against testsuite usage: `catch(`/`new(`/
// `sscanf(`/`parse_command(` are tight in every real call site (486, 131,
// 27, 2 occurrences respectively; exactly one spaced `catch (` site
// exists in the pristine corpus -- 486:1).
const CALL_LIKE_KEYWORDS = new Set(['catch', 'new', 'sscanf', 'parse_command']);

// Array-type-suffix '*' (`int *a`, `mixed ref *arr`, `int *a, *b;`) stays
// tight against the identifier that follows it; ordinary multiplication
// (`a * b`) is unaffected since a bare value token never satisfies this.
// `,` qualifies too for multi-declarator lists (`string *rows, *legend;`)
// -- LPC has no unary '*', so a bare '*' right after a comma can only be
// a repeated declarator, never an expression.
function isArrayTypeStarPrefix(tok) {
  return !!tok && (tok.kind === 'type' ||
                   (tok.kind === 'keyword' && tok.text === 'ref') ||
                   tok.text === '&' || tok.text === '*' || tok.text === ',');
}

// Unary '-'/'+' (`return -1`, `catch(v = -1)`) stay tight against their
// operand; binary +/- (`a - b`) keep normal spacing. Distinguished by
// what precedes the operator: an expression-ending token (value, `)`,
// `]`, `}`) means binary: anything else (another operator, an opening
// bracket, a comma, a keyword like `return`/`case`, or nothing at all)
// means unary. Confirmed against the pristine corpus: no spaced
// `= - 1`-shaped unary form exists at all (444:0 token-level). '++'/'--' count
// as expression-ending too: in every position where they can precede
// another operator or close in valid LPC they are POSTFIX (`i++ + j`,
// `j--)`) -- a prefix one is always followed directly by its lvalue.
// Also used to tell a POSTFIX '++'/'--' (tight before, keeps normal
// spacing after) from a PREFIX one (spaced before, tight against its
// operand) -- see the two positional rules in renderLine.
function endsExpression(tok) {
  return !!tok && (tok.kind === 'number' || tok.kind === 'identifier' || tok.kind === 'string' ||
                   tok.kind === 'char' || tok.kind === 'template' || tok.kind === 'textblock' ||
                   tok.text === ')' || tok.text === ']' || tok.text === '}' ||
                   tok.text === '++' || tok.text === '--');
}

// Bracket-and-comma-aware line wrapping. Operates purely on a token slice
// (no source-position dependence), so it's deterministic and idempotent:
// re-feeding the wrapped output through the tokenizer yields the identical
// token sequence, hence the identical wrap decision.
//
// Returns fully-rendered (indented) line strings. If a line fits under
// `printWidth` it's returned as-is; otherwise the OUTERMOST bracket pair
// that has at least one comma directly inside it (not nested deeper) is
// split one element per line, and the header/each-element/trailer are
// recursively wrapped the same way. A line with no such splittable group
// (e.g. a long operator chain with no call/literal to break on) is left
// long rather than risk an unsafe or non-idempotent break.
function wrapTokens(toks, depth, printWidth, indentUnit, mappingContext = false, pendingTernary = 0,
                    dedentAt = undefined) {
  const rendered = indentUnit.repeat(depth) + renderLine(toks, mappingContext, pendingTernary);
  // The wrap decision measures the longest PHYSICAL line, with two
  // exclusions:
  //  - a TRAILING comment never counts: a line comment stays at the end
  //    of its line no matter how long it makes it -- restructuring the
  //    CODE because of a comment's length would move the comment
  //    relative to what it annotates;
  //  - a token's own EMBEDDED newlines (a multi-line string, template,
  //    or heredoc rendered verbatim) split the measurement -- the
  //    token's interior lines are what they are, and its total character
  //    count must never trigger wrapping of the code around it.
  let codeEnd = toks.length;
  while (codeEnd > 0 && toks[codeEnd - 1].kind === 'comment') codeEnd--;
  const measuredText = codeEnd === toks.length ? rendered
    : indentUnit.repeat(depth) + renderLine(toks.slice(0, codeEnd), mappingContext, pendingTernary);
  let measured = 0;
  for (const physLine of measuredText.split('\n')) {
    if (physLine.length > measured) measured = physLine.length;
  }
  if (toks.length === 0 || measured <= printWidth) return [rendered];

  const pairs = matchedPairs(toks);
  for (const p of pairs) {
    const openTok = toks[p.open];
    // Never split an INDEX subscript (`x[a, b]` -- a comma expression):
    // index frames don't preserve line breaks on re-parse, so the split
    // would collapse again next pass. Mapping literals (`([`) do split.
    if (openTok.text === '[' &&
        !(p.open > 0 && prevNonComment(toks, p.open).text === '(')) continue;
    // Never split a one-lined BLOCK pair or anything inside one -- the
    // one-liner decision was made from the source's own layout; a wrap
    // split inside it renders a half-expanded hybrid the re-parse then
    // reflows differently. (Declarator commas are a block pair's own
    // direct commas; nested calls are pairs inside it.)
    if (openTok.text === '{' &&
        !(p.open > 0 && prevNonComment(toks, p.open).text === '(')) continue;
    let insideBlock = false;
    for (const q of pairs) {
      if (q.open < p.open && q.close > p.close && toks[q.open].text === '{' &&
          !(q.open > 0 && prevNonComment(toks, q.open).text === '(')) {
        insideBlock = true;
        break;
      }
    }
    if (insideBlock) continue;
    const commas = directCommas(toks, p.open, p.close);
    if (commas.length === 0) continue;

    const segments = [];
    let segStart = p.open + 1;
    for (const c of commas) {
      segments.push(toks.slice(segStart, c + 1)); // include the comma itself
      segStart = c + 1;
    }
    segments.push(toks.slice(segStart, p.close)); // trailing (comma-less) segment
    const nonEmpty = segments.filter((s) => s.length > 0);
    if (nonEmpty.length < 2) continue;

    // Whether EACH ELEMENT segment is itself a mapping's key:value pair
    // depends on whether `p` (the bracket actually being split here) is
    // the mapping's own '[...]', not on whatever `mappingContext` this
    // wrapTokens call inherited from its caller -- e.g. splitting a call's
    // long single-line argument list (`insert((["a": 1, "b": 2]));`) that
    // happens to sit at the top level (not itself inside another mapping)
    // must still recognize each element's OWN colon once the split
    // reaches the mapping's brackets, regardless of the outer context.
    const isMappingPair = toks[p.open].text === '[' && p.open > 0 && toks[p.open - 1].text === '(';
    const segMappingContext = isMappingPair || mappingContext;
    // The depth a SECOND pass's main loop will derive for this pair's
    // elements: a token BEFORE the pair that closes a bumped multiline
    // frame from an earlier line dedents it (the main loop defers those
    // dedents to after the line flushes -- flush() marks exactly those
    // close tokens in `dedentAt`). Without this, splitting a line like
    // `(: $1 :)) - ({ ".." })) {` indents the array's elements one level
    // deeper than the re-parse derives -- a narrow-printWidth idempotency
    // break (found by the width sweep). Unmatched OPENS contribute
    // nothing: the main loop bumps only the frame that is TOP at a line
    // break, and any break inside such an open happens while a deeper
    // frame is top.
    let eff = depth;
    if (dedentAt) {
      for (let k = 0; k < p.open; k++) {
        if (dedentAt.has(toks[k])) eff = Math.max(0, eff - 1);
      }
    }
    const out = [];
    out.push(...wrapTokens(toks.slice(0, p.open + 1), depth, printWidth, indentUnit, mappingContext,
                           pendingTernary, dedentAt));
    for (const seg of nonEmpty) {
      // capped like the main loop's multiline bumps, so a pair past the
      // cap gets the same depth the re-parse will derive
      out.push(...wrapTokens(seg, Math.min(eff + 1, MAX_MULTILINE_INDENT), printWidth, indentUnit,
                             segMappingContext, 0, dedentAt));
    }
    // The trailer starts after the split pair's close, but a ternary can
    // SPAN the split (`"k": cond ? funcall(...) : z,` splits at funcall's
    // parens, leaving `): z,` in the trailer with its '?' back in the
    // header) -- carry the header's still-pending '?' count in so the
    // trailer's ':' is recognized as the ternary's, not (inside a mapping
    // element) mistaken for the key colon and rendered tight.
    out.push(...wrapTokens(toks.slice(p.close), eff, printWidth, indentUnit, mappingContext,
                           pendingTernaryCount(toks, 0, p.open, pendingTernary), dedentAt));
    return out;
  }
  return [rendered];
}

// How many ternary '?'s opened at THIS slice's own bracket level are still
// unmatched by a ':' over toks[from, to) -- the count a renderLine (or a
// later wrapTokens trailer slice) should start from. `initial` carries the
// count already pending when the slice begins. ';' and top-level ',' both
// hard-reset (a ternary never spans either); unmatched closes clamp back
// to level 0 (a trailer slice legitimately starts with the closes of
// brackets opened in earlier slices).
function pendingTernaryCount(toks, from, to, initial = 0) {
  let level = 0;
  let count = initial;
  for (let i = from; i < to; i++) {
    const c = toks[i].text;
    if (NEST_OPEN.has(c)) level++;
    else if (NEST_CLOSE.has(c)) level = Math.max(0, level - 1);
    else if (level === 0) {
      if (c === '?') count++;
      else if (c === ',' || c === ';') count = 0;
      else if (c === ':' && count > 0) count--;
    }
  }
  return count;
}

// All matched (), {}, [] pairs in `toks`, outermost-opening-first.
// Deliberately excludes '(:'/':)' (functional literal delimiters): the
// MAIN formatLPC loop's litStack has no frame kind for them (unlike
// '('/'{'/'['), so a functional literal split across lines here wouldn't
// be recognized as "already multiline" on a second pass and would
// re-indent differently -- a real idempotency regression, confirmed
// empirically. A functional literal long enough to need splitting is
// left as one long line instead (safe, matching the "long operator chain
// with no splittable bracket" limitation already documented).
function matchedPairs(toks) {
  const OPEN = new Set(['(', '{', '[']);
  const CLOSE = new Set([')', '}', ']']);
  const stack = [];
  const pairs = [];
  for (let i = 0; i < toks.length; i++) {
    const c = toks[i].text;
    if (OPEN.has(c)) stack.push(i);
    else if (CLOSE.has(c)) {
      const open = stack.pop();
      if (open !== undefined) pairs.push({ open, close: i });
    }
  }
  pairs.sort((a, b) => a.open - b.open);
  return pairs;
}

// Comma indices directly inside (open, close) -- not inside a deeper nested
// bracket pair (so a literal's own '{'/'[' commas aren't seen as the outer
// '(' pair's commas, and a nested call's args aren't seen as the outer
// call's args).
function directCommas(toks, open, close) {
  // NEST_OPEN/NEST_CLOSE include the functional-literal delimiters --
  // without them, a comma inside `(: a, b :)` would read as depth-0
  // relative to whatever call it's an argument of and get mistaken for
  // that call's OWN argument separator.
  let localDepth = 0;
  const commas = [];
  for (let i = open + 1; i < close; i++) {
    const c = toks[i].text;
    if (NEST_OPEN.has(c)) { localDepth++; continue; }
    if (NEST_CLOSE.has(c)) { localDepth--; continue; }
    if (c === ',' && localDepth === 0) commas.push(i);
  }
  return commas;
}

// Would rendering `prev` and `t` with no space between them change what a
// re-tokenize sees? True when the concatenation's first lexeme is not
// exactly `prev`'s last lexeme -- i.e. the boundary moved. `prev` can be a
// multi-lexeme token (a 'verbatim' stringize span like `1+2`), so compare
// against its LAST lexeme, not its whole text. Results are cached: the set
// of distinct adjacent-pair spellings in real code is tiny.
const mergeCheckCache = new Map();
function tokensWouldMerge(prev, t) {
  const key = prev.text + '\u0000' + t.text;
  let v = mergeCheckCache.get(key);
  if (v === undefined) {
    // Only reason about `prev` if its text re-lexes in ISOLATION as
    // exactly one token equal to itself. A context-dependent token (a
    // template SEGMENT like '}!`' -- the tokenizer splits `a${x}b` around
    // each interpolation, and a segment only lexes correctly with the
    // template state built up by earlier tokens) or a multi-lexeme
    // 'verbatim' stringize span doesn't satisfy that, and this check
    // can't reason about its boundary -- report "no merge" rather than
    // inserting spurious spaces after every template. All the dangerous
    // merge pairs are single-lexeme operators/punctuation/numbers, which
    // always round-trip.
    const solo = tokenize(prev.text).filter((x) => x.kind !== 'whitespace');
    if (solo.length !== 1 || solo[0].text !== prev.text) {
      v = false;
    } else {
      const joined = tokenize(prev.text + t.text).filter((x) => x.kind !== 'whitespace');
      v = joined.length === 0 || joined[0].text !== prev.text;
    }
    mergeCheckCache.set(key, v);
  }
  return v;
}

// `mappingContext`: true when this token slice is itself one element of a
// multi-line mapping literal (`([ "a": 1,\n  "b": 2,\n])` -- each element
// flushes as its OWN renderLine call, via `flush`'s own litStack check, so
// `bracketStack` below never sees the '[' that opened the mapping -- it's
// on an earlier, already-flushed line). Used only as a fallback for the
// mapping-colon rule when bracketStack has no LOCAL bracket to consult.
// `pendingTernary`: how many ternary '?'s from EARLIER slices of the same
// logical line are still awaiting their ':' when this slice begins (see
// pendingTernaryCount) -- seeds the ternaryStack's permanent bottom frame
// so a ':' whose '?' sits in a previous slice is still recognized as a
// ternary's.
function renderLine(toks, mappingContext = false, pendingTernary = 0) {
  let out = '';
  // Tiny bracket-context stack, local to this token slice: only tracks
  // whether the immediately-enclosing '(' is a `new(...)` call, for the
  // class-member-initializer colon rule below. Symmetric push/pop across
  // every bracket type keeps it from desyncing on nested brackets; an
  // unbalanced leading close (a wrapTokens trailer slice) just no-ops.
  // Deliberately starts and can return to EMPTY (no seed) -- the tail-
  // close rule below depends on that.
  const bracketStack = [];
  // Parallel to bracketStack but with one extra PERMANENT bottom frame
  // (never popped) so it always has somewhere to count into, even for a
  // ternary at this slice's own top level (bracketStack itself is 0 deep
  // there, e.g. a wrapped argument segment like `cond ? x : y,` that's
  // semantically nested but locally top-level to this renderLine call).
  // Counts unmatched '?' seen since the last ',' at the current depth, so
  // a ':' can tell a ternary's own colon (`cond ? a : b` -- keeps its
  // space, decrements the count) from a mapping's key:value colon
  // (`([ "a" : cond ? x : y ])` -- no pending '?' yet when the KEY's
  // colon is reached) even when a ternary appears as a mapping's value,
  // without a real expression parser.
  const ternaryStack = [pendingTernary];
  // True while the LAST token processed was a ')' that closed a control-
  // flow condition's paren / a cast's paren -- consulted by the
  // '('-tight and cast-operand rules above.
  let closedCFParen = false;
  let closedCastParen = false;
  // True while a `case <expr>` / `default` label is open and its ':' has
  // not been rendered yet. Set on the KEYWORD (not inferred from
  // toks[0], which both missed a label mid-line -- a one-lined
  // `switch (x) { case 1: break; }` -- and falsely captured EVERY later
  // colon on a merged `case 1: y = a ? b : c;` line, tightening the
  // ternary's colon); cleared by the label colon itself and at statement
  // boundaries. The ternary guard keeps `case a: x = b ? c : d;`'s
  // ternary colon spaced even while a later label on the same line could
  // re-arm the flag.
  let caseLabelOpen = false;
  for (let i = 0; i < toks.length; i++) {
    const t = toks[i];
    const prev = toks[i - 1];
    let thisClosesCF = false;
    let thisClosesCast = false;
    let sep = ' ';
    if (i === 0) sep = '';
    // A TRAILING comment (nothing but comments after it on the line)
    // keeps the SOURCE's gap before it, with a two-space minimum for
    // '//' comments -- src/.clang-format's Google base sets
    // SpacesBeforeTrailingComments: 2, and preserving a wider source gap
    // keeps hand-aligned trailing-comment columns aligned (the pristine
    // corpus's >=2-space gaps outnumber single-space ones ~6:1),
    // mirroring AlignTrailingComments without moving any comment. The
    // gap is source whitespace only when prev really was the same-line
    // neighbor; otherwise (a comment first on its rendered line hits
    // i === 0 above, never here) fall back to the minimum. Mid-line
    // comments with code after them keep the normal single space.
    else if (t.kind === 'comment' && prev && toks.slice(i).every((x) => x.kind === 'comment')) {
      const min = t.text.startsWith('//') ? 2 : 1;
      const srcGap = (t.line === prev.line && t.start > prev.end) ? t.start - prev.end : 1;
      sep = ' '.repeat(Math.max(min, srcGap));
    }
    // ...except after another ';' or before an empty clause's ')' -- the
    // pristine corpus spaces empty for-header clauses (`for (i = 0; ;
    // i++)`, `for (...; i < len; )`, 6:0).
    else if (NO_SPACE_BEFORE.has(t.text) &&
             !((t.text === ';' || t.text === ')') && prev && prev.text === ';')) sep = '';
    // '++'/'--' are tight-before only in POSTFIX position -- the previous
    // token must actually end an expression (`i++`, `a[0]--`, `f()++`).
    // In prefix position they keep the normal space (`return ++count;`,
    // `x = ++i;`, `for (;; ++i)`); tightness against their OWN operand is
    // the separate after-rule below.
    else if ((t.text === '++' || t.text === '--') && endsExpression(prev)) sep = '';
    // '...' hugs the expression it spreads (`mixed extra...`,
    // `f(args...)`, `foo(({ })...)` -- tight 79:6 in the pristine
    // corpus); the BARE varargs marker after a comma keeps its space
    // (`f(int a, ...)`).
    else if (t.text === '...' && endsExpression(prev)) sep = '';
    // See precedesQualifiedScope() above -- bare '::' (no left-hand
    // qualifier) keeps its normal leading space, falling through to the
    // default ' ' below.
    else if (t.text === '::' && precedesQualifiedScope(prev)) sep = '';
    // `case X:` / `default:` -- no space before the label colon (near-
    // universal house style; unlike the ternary and mapping-literal ':',
    // which do get a leading space).
    else if (t.text === ':' && caseLabelOpen &&
             ternaryStack[ternaryStack.length - 1] === 0) sep = '';
    // A modifier-section label's colon is tight too (`public:`,
    // `nomask private:` -- grammar.y's `type_modifier_list ':'`,
    // applying the modifiers to all following declarations); a modifier
    // can never be a ternary/mapping operand, so prev-is-modifier is
    // unambiguous.
    else if (t.text === ':' && prev && prev.kind === 'modifier') sep = '';
    else if (t.text === ':' && bracketStack[bracketStack.length - 1]?.kind === 'new-call' &&
             ternaryStack[ternaryStack.length - 1] === 0) sep = '';
    // A mapping's key:value colon is tight before (`([ "a": 1 ])`) unless
    // this is actually a ternary's colon reached while INSIDE a mapping
    // literal (see ternaryStack above) -- that keeps its normal space.
    // The `mappingContext` fallback covers a multi-line mapping element
    // rendered as its own slice (see the function comment above).
    else if (t.text === ':' &&
             (bracketStack[bracketStack.length - 1]?.kind === 'mapping-bracket' ||
              (bracketStack.length === 0 && mappingContext)) &&
             ternaryStack[ternaryStack.length - 1] === 0) sep = '';
    // A DEFAULT-ARGUMENT colon in a declaration's parameter list
    // (`string b: (: "str" :)` -- grammar's `name ':' expr` parameter
    // form, tight-before at every pristine-corpus site) -- the parameter
    // NAME precedes it, and a pending '?' means the colon is a ternary's
    // instead. Covers both the in-parens case and a printWidth-wrapped
    // parameter rendered as its own slice (empty local bracketStack, not
    // a mapping element).
    else if (t.text === ':' && prev && prev.kind === 'identifier' &&
             ternaryStack[ternaryStack.length - 1] === 0 &&
             (bracketStack[bracketStack.length - 1]?.kind === 'other' ||
              (bracketStack.length === 0 && !mappingContext))) sep = '';
    // Template interpolation hugs its expression: nothing after the
    // opening `${` and nothing before the closing/next `}...` segment
    // (`\`${name}\``, tight roughly 20:3 on each side in the pristine corpus). The
    // interpolation's INTERIOR tokens keep normal spacing
    // (`\`${ a + b }\`` renders `\`${a + b}\``).
    else if (prev && prev.kind === 'template' && prev.text.endsWith('${')) sep = '';
    else if (t.kind === 'template' && t.text.startsWith('}')) sep = '';
    // An empty '{}' pair is tight regardless of kind -- an empty array
    // literal (`({})`, the pristine corpus's dominant convention at
    // roughly 3:1) AND a control-flow block (`void create() {}`; pristine
    // blocks actually lean `{ }` 2.6:1, but tight-empty across ALL pair
    // kinds is the deliberate, user-directed house style here). A NON-empty array literal still gets
    // the inner padding space (`({ 1, 2, 3 })`, falls through below).
    else if (t.text === '}' && prev && prev.text === '{') sep = '';
    // An empty '[]' mapping literal (`([])`) is likewise tight -- padded
    // `([ ])` is the minority spelling in the pristine corpus (roughly
    // 1:7). A NON-empty mapping still gets the inner padding space
    // (falls through to the default ' ').
    else if (t.text === ']' && prev && prev.text === '[' &&
             bracketStack[bracketStack.length - 1]?.kind === 'mapping-bracket') sep = '';
    // A '}'/']' at EMPTY bracketStack closes a pair that opened on an
    // earlier already-flushed line. Directly after a same-kind close it
    // stays tight (`]]` 25:0, `}}`); after a ')' it keeps a space --
    // nested literal closes come as `X)` pairs and the pristine corpus
    // pads between the pairs (`2 }) ]) });`, spaced-to-tight roughly
    // 9:1). A '}' after ']' keeps its space too (an index close then an
    // array literal's close, `s[0] });` -- spaced 6:1). After real
    // content (`"b": 2 ])`, a close the source glued onto the last
    // element) it falls through to the literal's normal inner padding.
    else if ((t.text === '}' || t.text === ']') && bracketStack.length === 0 && prev &&
             (prev.text === '}' || prev.text === ']') &&
             !(t.text === '}' && prev.text === ']')) sep = '';
    // ']' closing an index (`a[0]`) stays tight; closing a NON-empty
    // mapping literal (`([ "a": 1 ])`) gets the same inner padding as '{'/
    // '}' does for a non-empty array literal (falls through to the
    // default ' ' -- the empty case was already handled above).
    else if (t.text === ']' && bracketStack[bracketStack.length - 1]?.kind === 'index-bracket') sep = '';
    // A '<' acting as a MARKER, not less-than, stays tight on its far
    // side: `..<` (exclusive range bound, `a[..<j]`) and `[<n]` (reverse
    // index -- tight forms outnumber spaced ones better than 30:1 in the
    // corpus). Recognized by what precedes the '<'; bare less-than
    // elsewhere keeps its space.
    else if (prev && prev.text === '<' && toks[i - 2] &&
             (toks[i - 2].text === '..' || toks[i - 2].text === '[')) sep = '';
    // '[' opening an index stays tight after it; opening a mapping
    // literal gets a padding space, same as '{' does for array literals
    // (falls through to the default ' ').
    else if (prev && prev.text === '[' && bracketStack[bracketStack.length - 1]?.kind === 'index-bracket') sep = '';
    else if (prev && NO_SPACE_AFTER.has(prev.text)) sep = '';
    // Array-type-suffix '*' (`int *a`, `mixed ref *arr`) stays tight
    // against the identifier that follows it; ordinary multiplication
    // (`a * b`) is unaffected since a bare identifier never satisfies
    // isArrayTypeStarPrefix. Deliberately the OPPOSITE binding from
    // src/.clang-format's `PointerAlignment: Left` (`type* name`): LPC's
    // '*' is the array marker, not a pointer, and the pristine corpus
    // writes `type *name` over `type* name` roughly 5:1 (362:69).
    else if (prev && prev.text === '*' && isArrayTypeStarPrefix(toks[i - 2])) sep = '';
    // Unary '-'/'+' stays tight against its operand; binary +/- keeps its
    // space (endsExpression(toks[i - 2]) is false for a unary operator,
    // since something other than a value/`)`/`]`/`}` precedes it). A
    // string/template operand is excluded: LPC has no unary +/- on
    // strings, so a `+` before one is always the binary concatenation --
    // e.g. a continuation line starting `+ "bbb"`, whose true left
    // operand sits on the previous rendered line where this slice can't
    // see it.
    else if (prev && (prev.text === '-' || prev.text === '+') && !endsExpression(toks[i - 2]) &&
             t.kind !== 'string' && t.kind !== 'template') sep = '';
    // '(' is tight after anything CALLABLE: a name (`f(`), a type-named
    // macro, an index (`funcs[0](5, 5)`), a call's own result
    // (`f(a)(b)`), or a functional literal (`(: f :)(5)`). It keeps its
    // space after a control-flow keyword (`if (`) and after a control-
    // flow CONDITION's ')' (`if (x) (a = b);` -- tracked via 'cf-paren'
    // in bracketStack), where the '(' starts a new expression rather
    // than calling one.
    // (A ')' that closed a CAST is callable-adjacent even when its frame
    // was classified cf-paren -- `return (string)(expr)` stays tight,
    // matching the assignment-position `y = (string)(expr)`.)
    else if (prev && t.text === '(' &&
             (prev.kind === 'identifier' || prev.kind === 'type' || prev.kind === 'keyword' ||
              prev.text === ']' || prev.text === ':)' ||
              (prev.text === ')' && (!closedCFParen || closedCastParen))))
      sep = prev.kind === 'keyword' && !CALL_LIKE_KEYWORDS.has(prev.text) ? ' ' : '';
    // A cast's ')' is tight against its operand (`(string)"x"`,
    // `(object)tp`, `(string)new(...)`); a declaration prototype's
    // bare-type ')' never collides (what follows it is ';'/'{', which
    // this rule excludes).
    else if (prev && prev.text === ')' && closedCastParen &&
             (t.kind === 'identifier' || t.kind === 'string' || t.kind === 'number' ||
              t.kind === 'char' || t.kind === 'template' || t.kind === 'keyword')) sep = '';
    // A PREFIX '++'/'--' is tight against its operand (`++i`); after a
    // POSTFIX one (`i++ < n`, `j-- - k`) the next token keeps its normal
    // spacing. Same positional test as the tight-before rule above:
    // prefix means the token before the '++' does NOT end an expression.
    else if (prev && prev.kind === 'operator' && (prev.text === '++' || prev.text === '--') &&
             !endsExpression(toks[i - 2])) sep = '';
    // Keyword-tail safety net: a reserved WORD (keyword/type/modifier/
    // efunkw) never has the next token glued onto it, regardless of which
    // tight rule above fired -- `return++count`, `return-1`-shaped output
    // is always wrong even when it re-lexes identically. The only
    // legitimate tight followers are ';' (`return;`, `break;`), ':' (the
    // `default:` label), ','/')' (separators/closers never take a space
    // before them), '(' (the dedicated rule above already distinguishes
    // `catch(`/`new(`/`function(` from `if (`/`return (`), and a
    // qualified '::' (`efun::foo`).
    if (sep === '' && prev &&
        (prev.kind === 'keyword' || prev.kind === 'type' ||
         prev.kind === 'modifier' || prev.kind === 'efunkw') &&
        !(t.text === ';' || t.text === ':' || t.text === ',' || t.text === ')' ||
          t.text === '(' || t.text === '::')) {
      sep = ' ';
    }
    // Token-merge safety net: never butt two tokens together whose
    // concatenation re-lexes as something else -- `a - --b` must not
    // become `a ---b` (re-lexing as `(a--) - b`), `- -x` must not become
    // `--x` (a pre-decrement!), and `f( ::g() )` must not become
    // `f(::g())` (whose `(:` re-lexes as a functional-literal opener).
    // Longest-match lexing means ANY tight rule above can accidentally
    // manufacture a longer operator; checking against the real tokenizer
    // catches every such pair, present and future, in one place.
    if (sep === '' && prev && tokensWouldMerge(prev, t)) sep = ' ';
    out += sep + t.text;

    // Bracket KIND decisions look through comments (`( /*c*/ [ ...` is
    // still a mapping literal to the real lexer) -- only the kind
    // classification does; the spacing rules above keep using the raw
    // neighbor, comments render with normal spacing around them.
    // (Computed lazily: only the three bracket-open branches need it.)
    const prevCode = (t.text === '(' || t.text === '{' || t.text === '[')
      ? prevNonComment(toks, i) : undefined;
    if (t.text === '(') {
      bracketStack.push({
        kind: prevCode && prevCode.kind === 'keyword' && prevCode.text === 'new' ? 'new-call'
          : prevCode && prevCode.kind === 'keyword' && !CALL_LIKE_KEYWORDS.has(prevCode.text) ? 'cf-paren'
          : 'other',
        open: i,
      });
      ternaryStack.push(0);
    } else if (t.text === '{') {
      bracketStack.push({ kind: prevCode && prevCode.text === '(' ? 'array-brace' : 'block-brace' });
      ternaryStack.push(0);
    } else if (t.text === '[') {
      bracketStack.push({ kind: prevCode && prevCode.text === '(' ? 'mapping-bracket' : 'index-bracket' });
      ternaryStack.push(0);
    } else if (t.text === ')' || t.text === '}' || t.text === ']') {
      const frame = bracketStack[bracketStack.length - 1];
      thisClosesCF = t.text === ')' && frame?.kind === 'cf-paren';
      // A '(...)' whose entire content is one type token (plus an
      // optional array '*') is a CAST -- its close is tight against the
      // operand (`(string)"x"`, `(mixed *)arr`: tight outnumbers spaced
      // 25:1 in the pristine corpus). A cf-paren frame qualifies too:
      // the '(' of `return (string)x` follows the keyword and gets
      // classified cf-paren, but a bare type as a control-flow condition
      // isn't legal LPC, so single-type content is still a cast there
      // (`return (string)x` tight 5:0 pristine).
      thisClosesCast = t.text === ')' &&
        (frame?.kind === 'other' || frame?.kind === 'cf-paren') && frame.open !== undefined &&
        ((i - frame.open === 2 && toks[i - 1].kind === 'type') ||
         (i - frame.open === 3 && toks[i - 2].kind === 'type' && toks[i - 1].text === '*'));
      if (bracketStack.length) bracketStack.pop();
      // Never pop below the one permanent bottom frame (an unbalanced
      // leading close in a wrapTokens trailer slice would otherwise drain
      // it, leaving later '?'/':' bookkeeping with nowhere to count into).
      if (ternaryStack.length > 1) ternaryStack.pop();
      // Only '}' ends a label context -- a ')' can legitimately sit inside
      // one (`case (FOO):`, a parenthesized constant expression).
      if (t.text === '}') caseLabelOpen = false;
    } else if (t.text === '?') {
      ternaryStack[ternaryStack.length - 1]++;
    } else if (t.text === ',') {
      ternaryStack[ternaryStack.length - 1] = 0;
    } else if (t.kind === 'keyword' && (t.text === 'case' || t.text === 'default')) {
      caseLabelOpen = true;
    } else if (t.text === ':' && caseLabelOpen && ternaryStack[ternaryStack.length - 1] === 0) {
      caseLabelOpen = false; // the label's own colon, just rendered tight
    } else if (t.text === ':' && ternaryStack[ternaryStack.length - 1] > 0) {
      ternaryStack[ternaryStack.length - 1]--;
    } else if (t.text === ';') {
      caseLabelOpen = false;
    }
    closedCFParen = thisClosesCF;
    closedCastParen = thisClosesCast;
  }
  return out;
}
