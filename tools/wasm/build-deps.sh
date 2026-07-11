#!/usr/bin/env bash
#
# build-deps.sh -- cross-build the WASM driver's static dependencies
# (ICU and zlib) with Emscripten, into a single install prefix.
#
# Produces under $PREFIX (default /opt/wasm-deps):
#   lib/libicuuc.a lib/libicui18n.a lib/libicudata.a lib/libz.a
#   include/unicode/*.h include/zlib.h ...
#
# Requirements: emcc/emconfigure/emmake on PATH, a native C/C++
# toolchain (ICU cross-builds need native ICU tools first), curl.
#
# Environment overrides:
#   PREFIX     install prefix            (default /opt/wasm-deps)
#   WORK       scratch build directory   (default <prefix>-build)
#   ICU_VER    ICU release               (default 74-2)
#   ZLIB_VER   zlib release              (default 1.3.1)

set -euo pipefail

PREFIX=${PREFIX:-/opt/wasm-deps}
WORK=${WORK:-${PREFIX}-build}
ICU_VER=${ICU_VER:-74-2}
ZLIB_VER=${ZLIB_VER:-1.3.1}
NPROC=$(nproc 2>/dev/null || echo 4)

ICU_VER_U=${ICU_VER//-/_}   # 74_2
ICU_MAJOR=${ICU_VER%%-*}    # 74

if [ -f "$PREFIX/lib/libicuuc.a" ] && [ -f "$PREFIX/lib/libicudata.a" ] && \
   [ -f "$PREFIX/lib/libz.a" ]; then
  echo "wasm deps already present in $PREFIX; nothing to do."
  exit 0
fi

mkdir -p "$PREFIX" "$WORK"
cd "$WORK"

# ---------------- zlib ----------------
if [ ! -f "$PREFIX/lib/libz.a" ]; then
  echo "=== zlib $ZLIB_VER ==="
  if [ ! -d "zlib-$ZLIB_VER" ]; then
    curl -fsSL -o zlib.tar.gz \
      "https://github.com/madler/zlib/releases/download/v$ZLIB_VER/zlib-$ZLIB_VER.tar.gz"
    tar xzf zlib.tar.gz
  fi
  (cd "zlib-$ZLIB_VER" &&
    emconfigure ./configure --static --prefix="$PREFIX" &&
    emmake make -j"$NPROC" libz.a &&
    emmake make install)
fi

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

  # 3. The data archive: generate C source for the .dat with the HOST
  #    genccode, compile it with emcc, archive it as libicudata.a.
  echo "=== ICU data (genccode -> emcc) ==="
  DAT=$(find "$PWD/build-host/data/out/tmp" -name "icudt${ICU_MAJOR}*.dat" | head -1)
  [ -n "$DAT" ] || { echo "error: host ICU data file not found" >&2; exit 1; }
  (cd "$WORK" &&
    "$WORK/icu/source/build-host/bin/genccode" -e "icudt${ICU_MAJOR}" -d . "$DAT" &&
    emcc -O2 -I"$WORK/icu/source/common" -c "$(basename "${DAT%.dat}")_dat.c" \
         -o icudata.o &&
    emar rcs "$PREFIX/lib/libicudata.a" icudata.o)
fi

echo
echo "wasm deps installed:"
ls -la "$PREFIX/lib"
