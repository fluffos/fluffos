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
# Every write is guarded by format-corpus.mjs: a file is only touched if
# the output is token-sequence-equivalent to the input and idempotent;
# anything else is reported and left alone, and the script exits nonzero.
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"

EXCLUDES=(
  "testsuite/single/tests/compiler/fail/bad_utf8_string.lpc"
  "testsuite/single/tests/compiler/fail/bad_utf8_arrayblock.lpc"
)

cd -- "${REPO_ROOT}"

find testsuite \( -name '*.lpc' -o -name '*.c' \) -type f | sort |
  grep -Fxv -e "${EXCLUDES[0]}" -e "${EXCLUDES[1]}" |
  node "${REPO_ROOT}/tools/lpc-syntax/bin/format-corpus.mjs" "$@"
