#!/bin/bash
# TEMPORARY DIAGNOSTIC -- minimal-reproducer matrix for the MinGW
# "every binary exits 3 after running correctly" teardown bug.
#
# Stage 1 uses only toolchain-provided sources, so a failure here is airtight
# proof that no fluffos code is involved. Runs in seconds -- do NOT make this
# depend on the ~10 min driver build.
set +e
cd /tmp || exit 0
rm -rf minirepro && mkdir -p minirepro && cd minirepro || exit 0

cat > empty.c <<'EOF'
int main(void) { return 0; }
EOF

cat > empty.cc <<'EOF'
int main(void) { return 0; }
EOF

cat > hello.cc <<'EOF'
#include <cstdio>
int main(void) { printf("hi\n"); return 0; }
EOF

cat > printf_float.cc <<'EOF'
#include <cstdio>
/* printf("%f") pulls in gdtoa, whose lock cleanup became an
   __attribute__((destructor)) in mingw-w64 crt r409. */
int main(void) { printf("%f\n", 1.5); return 0; }
EOF

cat > static_dtor.cc <<'EOF'
#include <cstdio>
#include <string>
#include <vector>
static std::vector<std::string> g{"alpha", "beta", "gamma"};
int main(void) { printf("%zu\n", g.size()); return 0; }
EOF

# Exactly what src/CMakeLists.txt produces for a Debug MinGW build, taken
# verbatim from a real CI compile+link line.
FLUFFOS_CXXFLAGS="-O0 -g3 -std=c++17 -fvisibility=hidden -DWINVER=0x0601 \
-D_WIN32_WINNT=0x0601 -D_UNICODE -DUNICODE -fno-omit-frame-pointer -D_GNU_SOURCE \
-fno-strict-aliasing -D_GLIBCXX_ASSERTIONS -D_FORTIFY_SOURCE=2 -fstack-protector-all \
-fsigned-char -fwrapv"
FLUFFOS_LDFLAGS="-pie -static -static-libgcc -static-libstdc++ -Wl,-Bstatic \
-Wl,--major-image-version,0,--minor-image-version,0 -lssp"

pass=0
fail=0

run() {
  local label="$1"; shift
  local src="$1"; shift
  local cc="$1"; shift
  rm -f probe.exe
  if ! $cc "$src" -o probe.exe "$@" 2> berr.txt; then
    printf '  %-34s BUILD FAILED: %s\n' "$label" "$(head -2 berr.txt | tr -d '\r\n')"
    return
  fi
  ./probe.exe > rout.txt 2> rerr.txt
  local rc=$?
  if [ "$rc" = 0 ]; then pass=$((pass+1)); else fail=$((fail+1)); fi
  printf '  %-34s exit=%-4s stdout=%-12s stderr=%s\n' \
    "$label" "$rc" "'$(tr -d '\r\n' < rout.txt)'" "'$(tr -d '\r\n' < rerr.txt)'"
}

echo "###################### toolchain ######################"
gcc --version | head -1
pacman -Q mingw-w64-x86_64-crt mingw-w64-x86_64-gcc \
          mingw-w64-x86_64-winpthreads mingw-w64-x86_64-headers 2>/dev/null

echo
echo "###################### STAGE 1: baseline ######################"
run "c/default"                empty.c        gcc
run "c/static"                 empty.c        gcc -static
run "cxx/default"              empty.cc       g++
run "cxx/hello-default"        hello.cc       g++
run "cxx/static"               empty.cc       g++ -static -static-libgcc -static-libstdc++

echo
echo "###################### STAGE 2: the full fluffos flag set ######################"
run "cxx/fluffos empty"        empty.cc       g++ $FLUFFOS_CXXFLAGS $FLUFFOS_LDFLAGS
run "cxx/fluffos hello"        hello.cc       g++ $FLUFFOS_CXXFLAGS $FLUFFOS_LDFLAGS
run "cxx/fluffos printf-float" printf_float.cc g++ $FLUFFOS_CXXFLAGS $FLUFFOS_LDFLAGS
run "cxx/fluffos static-dtor"  static_dtor.cc g++ $FLUFFOS_CXXFLAGS $FLUFFOS_LDFLAGS

echo
echo "###################### STAGE 3: one knob at a time on plain -static C++ ######################"
BASE="-static -static-libgcc -static-libstdc++"
run "  +pie"                   hello.cc       g++ $BASE -pie
run "  +lssp"                  hello.cc       g++ $BASE -lssp
run "  +fstack-protector-all"  hello.cc       g++ $BASE -fstack-protector-all
run "  +_FORTIFY_SOURCE=2"     hello.cc       g++ $BASE -D_FORTIFY_SOURCE=2 -O1
run "  +_GLIBCXX_ASSERTIONS"   hello.cc       g++ $BASE -D_GLIBCXX_ASSERTIONS
run "  +fvisibility=hidden"    hello.cc       g++ $BASE -fvisibility=hidden
run "  +g3"                    hello.cc       g++ $BASE -g3 -O0
run "  +UNICODE"               hello.cc       g++ $BASE -D_UNICODE -DUNICODE
run "  +image-version"         hello.cc       g++ $BASE -Wl,--major-image-version,0,--minor-image-version,0
run "  +out-implib"            hello.cc       g++ $BASE -Wl,--out-implib,libprobe.dll.a
run "  +pie+g3"                hello.cc       g++ $BASE -pie -g3 -O0
run "  +Bstatic"               hello.cc       g++ $BASE -Wl,-Bstatic

echo
echo "###################### STAGE 4: dynamic (non -static) variants ######################"
run "cxx/dynamic+pie"          hello.cc       g++ -pie
run "cxx/static-libstdc++only" hello.cc       g++ -static-libstdc++ -static-libgcc

echo
echo "###### STAGE 1-4 SUMMARY: exit0=$pass  nonzero=$fail ######"
echo
echo "###################### STAGE 5: gdb on the smallest failing probe ######################"
# Rebuild the simplest variant that fails (if any) and arm SIGABRT properly.
g++ $FLUFFOS_CXXFLAGS $FLUFFOS_LDFLAGS hello.cc -o probe.exe 2>/dev/null
if [ -f probe.exe ]; then
  ./probe.exe >/dev/null 2>&1
  if [ $? != 0 ]; then
    echo "(minimal fluffos-flags probe reproduces it -- running gdb)"
    gdb -batch -nx \
        -ex 'set confirm off' -ex 'set pagination off' \
        -ex 'set breakpoint pending on' \
        -ex 'handle SIGABRT stop print nopass' \
        -ex 'break abort' -ex 'break _exit' -ex 'break exit' \
        -ex 'break raise' -ex 'break ExitProcess' -ex 'break TerminateProcess' \
        -ex run \
        -ex 'echo \n==== stop 1 ====\n' -ex 'bt' -ex 'info args' -ex continue \
        -ex 'echo \n==== stop 2 ====\n' -ex 'bt' -ex 'info args' -ex continue \
        -ex 'echo \n==== stop 3 ====\n' -ex 'bt' -ex 'info args' -ex continue \
        -ex 'echo \n==== stop 4 ====\n' -ex 'bt' -ex 'info args' -ex continue \
        ./probe.exe 2>&1 | tail -80
  else
    echo "(minimal fluffos-flags probe exits 0 -- the trigger is NOT the flags;"
    echo " it is something linked into every fluffos binary. See stage 6.)"
  fi
fi
exit 0
