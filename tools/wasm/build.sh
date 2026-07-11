#!/usr/bin/env bash
#
# Build the FluffOS web driver (WebAssembly, via Emscripten) end to end
# and package the bundled testsuite mudlib as a demo. See
# docs/build-wasm.md for the full workflow and src/wasm/README.md for
# the architecture.
#
#   1. `native-tools` CMake preset: codegen binaries that must run on the
#      build machine (make_func, build_applies, make_options_defs).
#   2. `wasm` CMake preset through emcmake: the driver itself.
#   3. tools/wasm/pack-mudlib.sh: mudlib image + web shell -> dist.
#
# Prerequisites: emscripten on PATH, wasm-built ICU/zlib (run
# tools/wasm/build-deps.sh once), native toolchain + OpenSSL/ICU headers
# for stage 1.
#
# Usage: tools/wasm/build.sh [dist-dir]
#   MUDLIB=<dir> CONFIG=<relpath>  package a different mudlib
#   WASM_DEPS=<prefix>             non-default deps prefix

set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
MUDLIB=${MUDLIB:-$ROOT/testsuite}
CONFIG=${CONFIG:-etc/config.test}
DIST=${1:-$ROOT/build-wasm/dist}
NPROC=$(nproc 2>/dev/null || echo 4)

echo "=== Stage 1: native host tools (preset: native-tools) ==="
cmake --preset native-tools
cmake --build --preset native-tools -j"$NPROC"

echo "=== Stage 2: wasm driver (preset: wasm) ==="
emcmake cmake --preset wasm ${WASM_DEPS:+-DFLUFFOS_WASM_DEPS="$WASM_DEPS"}
cmake --build --preset wasm -j"$NPROC"

echo "=== Stage 3: package mudlib ==="
"$ROOT/tools/wasm/pack-mudlib.sh" \
  --mudlib "$MUDLIB" --config "$CONFIG" --out "$DIST"
