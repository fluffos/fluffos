#!/usr/bin/env bash
#
# build-deps.sh -- cross-build the WASM driver's static dependency (ICU)
# with Emscripten, into an install prefix. (zlib is not used on the wasm
# target: MCCP, the compress package and gzip'd file support are off.)
#
# Produces under $PREFIX (default /opt/wasm-deps):
#   lib/libicuuc.a lib/libicui18n.a lib/libicudata.a
#   include/unicode/*.h ...
#
# Requirements: emcc/emconfigure/emmake on PATH, a native C/C++
# toolchain (ICU cross-builds need native ICU tools first), curl.
#
# Environment overrides:
#   PREFIX     install prefix            (default /opt/wasm-deps)
#   WORK       scratch build directory   (default <prefix>-build)
#   ICU_VER    ICU release               (default 74-2)
#   ICU_KEEP   extra icupkg keep-patterns for the ICU data trim, space
#              separated (default: none beyond the built-ins below)
#
# ICU data: the stock icudt archive is ~30MB, which would dominate the
# wasm binary. The driver only needs break iteration (grapheme/line
# breaking) from the data archive -- character properties and NFC live
# inside libicuuc, and UTF-8/UTF-16/Latin-1/ASCII converters are
# algorithmic. The archive is therefore trimmed with icupkg down to
# brkitr rules + the converter alias table (<1MB); dictionary-based
# segmentation (CJK/Thai word breaking) and table charsets (GBK, Big5,
# ...) are dropped -- string_encode()/set_encoding() to those raise an
# LPC error on wasm, and LPC can test __WASM__ to adapt. Need a charset
# back? Re-run with the prefix removed and e.g.
#   ICU_KEEP='ibm-1386_P100-2001.cnv windows-936-2000.cnv'
# (icupkg -l on the .dat lists the item names).

set -euo pipefail

PREFIX=${PREFIX:-/opt/wasm-deps}
WORK=${WORK:-${PREFIX}-build}
ICU_VER=${ICU_VER:-74-2}
NPROC=$(nproc 2>/dev/null || echo 4)

ICU_VER_U=${ICU_VER//-/_}   # 74_2
ICU_MAJOR=${ICU_VER%%-*}    # 74

if [ -f "$PREFIX/lib/libicuuc.a" ] && [ -f "$PREFIX/lib/libicudata.a" ]; then
  echo "wasm deps already present in $PREFIX; nothing to do."
  exit 0
fi

mkdir -p "$PREFIX" "$WORK"
cd "$WORK"

# ---------------- ICU ----------------
if [ ! -f "$PREFIX/lib/libicuuc.a" ] || [ ! -f "$PREFIX/lib/libicudata.a" ]; then
  echo "=== ICU $ICU_VER ==="
  if [ ! -d icu ]; then
    curl -fsSL -o icu.tgz \
      "https://github.com/unicode-org/icu/releases/download/release-$ICU_VER/icu4c-$ICU_VER_U-src.tgz"
    tar xzf icu.tgz
  fi

  cd icu/source

  # ICU's config.sub doesn't know the emscripten triple; the generic
  # "unknown" platform with the linux Makefile fragment works fine.
  cp config/mh-linux config/mh-unknown

  # 1. Native host build: the cross build borrows its code-generation
  #    tools (and we use its genccode for the data archive below).
  if [ ! -f build-host/bin/genccode ]; then
    mkdir -p build-host
    (cd build-host &&
      CFLAGS=-O1 CXXFLAGS="-O1 -std=c++17" ../configure \
        --disable-tests --disable-samples --disable-extras >/dev/null &&
      make -j"$NPROC" >/dev/null)
  fi

  # 2. WASM cross build of the libraries. The `data` packaging step cannot
  #    produce wasm objects (pkgdata only knows native object formats), so
  #    the top-level make is allowed to fail after the libraries are
  #    built, and common/i18n are installed explicitly.
  mkdir -p build-wasm
  (cd build-wasm &&
    ICU_FLAGS="-O2 -DU_CHARSET_IS_UTF8=1" &&
    CFLAGS="$ICU_FLAGS" CXXFLAGS="$ICU_FLAGS -std=c++17" \
    emconfigure ../configure \
      --host=wasm32-unknown-none \
      --with-cross-build="$PWD/../build-host" \
      --enable-static --disable-shared \
      --disable-tests --disable-samples --disable-extras --disable-tools \
      --disable-dyload \
      --with-data-packaging=static \
      --prefix="$PREFIX" >/dev/null &&
    { emmake make -j"$NPROC" >/dev/null 2>&1 || true; } &&
    emmake make -C common install >/dev/null &&
    emmake make -C i18n install >/dev/null)

  # 3. The data archive: trim it with icupkg (see header -- the stock
  #    archive is ~30MB; break iteration + the converter alias table is
  #    all the driver needs), then generate C source for it with the
  #    HOST genccode, compile with emcc, archive as libicudata.a.
  #    (ICU_DATA_FILTER_FILE would be nicer, but it only applies when
  #    building the data from source -- the -src tarball ships a
  #    prebuilt .dat, so filter the .dat itself.)
  echo "=== ICU data (icupkg trim -> genccode -> emcc) ==="
  DAT=$(find "$PWD/build-host/data/out/tmp" -name "icudt${ICU_MAJOR}*.dat" | head -1)
  [ -n "$DAT" ] || { echo "error: host ICU data file not found" >&2; exit 1; }
  ICUPKG="$WORK/icu/source/build-host/bin/icupkg"
  (mkdir -p "$WORK/data-trim" && cd "$WORK/data-trim" &&
    cp "$DAT" . &&
    KEEP='^(brkitr/.*\.(brk|res)$|cnvalias\.icu$|pnames\.icu$|root\.res$|res_index\.res$|pool\.res$)' &&
    { "$ICUPKG" -l "$DAT" | grep -vE "$KEEP"
      # dictionary-based segmentation data is the next-largest chunk
      "$ICUPKG" -l "$DAT" | grep -E '^brkitr/.*\.dict$'; } | sort -u > remove-all.txt &&
    if [ -n "${ICU_KEEP:-}" ]; then
      printf '%s\n' $ICU_KEEP > keep-extra.txt
      grep -vFxf keep-extra.txt remove-all.txt > remove.txt
    else
      mv remove-all.txt remove.txt
    fi &&
    "$ICUPKG" -r remove.txt --ignore-deps "$(basename "$DAT")" &&
    echo "trimmed data archive: $(du -h "$(basename "$DAT")" | cut -f1)" &&
    "$WORK/icu/source/build-host/bin/genccode" -e "icudt${ICU_MAJOR}" -d . "$(basename "$DAT")" &&
    emcc -O2 -I"$WORK/icu/source/common" -c "$(basename "${DAT%.dat}")_dat.c" \
         -o icudata.o &&
    emar rcs "$PREFIX/lib/libicudata.a" icudata.o)
fi

echo
echo "wasm deps installed:"
ls -la "$PREFIX/lib"
