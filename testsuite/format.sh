#!/usr/bin/env bash
# Auto-format the testsuite LPC corpus (testsuite/**/*.lpc and *.c) with
# tools/lpc-syntax/format.mjs, in place. Dependency-free: needs only
# node (>= 18), no npm install.
#
#   testsuite/format.sh            # format in place
#   testsuite/format.sh --check    # exit 1 if anything is unformatted,
#                                  # write nothing (CI)
#
# EXCLUSIONS (do not remove): the two deliberately-malformed UTF-8
# compiler fixtures are raw byte fixtures, not text -- formatting (or
# even reading them as UTF-8 and writing back) would corrupt them.
# testsuite/.gitattributes marks them `binary` for the same reason; if a
# similar raw-invalid-byte fixture is added later, add it BOTH there and
# here. See AGENTS.md section 7.
#
# The three EOF-lexerror fixtures (eof_in_string / eof_in_comment /
# bad_at_block) are deliberately-unterminated sources whose brokenness
# is the point -- the driver hard-errors on them at lex time, and the
# formatter REFUSES such input by design (formatLPC throws; see
# docs/lpc/formatter.md "Safety guarantees"), so they must not be fed
# to it. If a new deliberately-unterminated fail fixture is added, add
# it here too.
#
# Every write is guarded by format-corpus.mjs: a file is only touched if
# the input lexes cleanly (no unterminated string/char/template/comment/
# text block) and the output is token-sequence-equivalent to the input
# and idempotent; anything else is reported and left alone, and the
# script exits nonzero.
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"

EXCLUDES=(
  "testsuite/single/tests/compiler/fail/bad_utf8_string.lpc"
  "testsuite/single/tests/compiler/fail/bad_utf8_arrayblock.lpc"
  "testsuite/single/tests/compiler/fail/eof_in_string.lpc"
  "testsuite/single/tests/compiler/fail/eof_in_comment.lpc"
  "testsuite/single/tests/compiler/fail/bad_at_block.lpc"
)

cd -- "${REPO_ROOT}"

EXCLUDE_ARGS=()
for x in "${EXCLUDES[@]}"; do EXCLUDE_ARGS+=(-e "${x}"); done

find testsuite \( -name '*.lpc' -o -name '*.c' \) -type f | sort |
  grep -Fxv "${EXCLUDE_ARGS[@]}" |
  node "${REPO_ROOT}/tools/lpc-syntax/bin/format-corpus.mjs" "$@"
