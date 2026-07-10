#!/bin/bash
# Regenerate the index.md files for the generated reference doc trees.
# Run from the docs/ directory: ./update_index.sh
#
# NOTE: lpc/index.md is hand-written — do not add lpc here.

set -e
cd "$(dirname "$0")"

./gen_index.py efun EFUN
./gen_index.py apply APPLY
./gen_index.py stdlib STDLIB
./gen_index.py cli cli
./gen_index.py concepts concepts
./gen_index.py driver driver

# Chinese docs (recurses into zh-CN/apply, zh-CN/efun, zh-CN/build)
./gen_index.py zh-CN zh-CN
