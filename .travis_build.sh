#!/bin/bash

if [[ "$(git branch | grep coverity_scan)" =~ ^.+coverity_scan$ ]]; then
  if [ -z "$COVERITY" ]; then
    echo "Only doing coverity scan in this branch, skipping this build"
    exit 0
  fi
else
  if [ -n "$COVERITY" ]; then
    echo "Skipping coverity on this branch."
    exit 0
  fi
fi

setup () {

case $COMPILER in
  gcc)
    export CXX="/usr/bin/g++"
    $CXX -v
    ;;
  clang)
    wget -q http://releases.llvm.org/4.0.0/clang+llvm-4.0.0-x86_64-linux-gnu-ubuntu-14.04.tar.xz
    tar axvf clang+llvm-*.tar.xz
    export CXX="$PWD/clang+llvm-4.0.0-x86_64-linux-gnu-ubuntu-14.04/bin/clang++ -Wno-error=unused-command-line-argument"
    $CXX -v
    ;;
esac

}

# do setup
setup

# stop on first error down below
set -eo pipefail

# testing part
cd src
./autogen.sh
cp local_options local_options.default
cp local_options.$CONFIG local_options

if [ -n "$GCOV" ]; then
  ./build.FluffOS $TYPE --enable-gcov=yes
else
  ./build.FluffOS $TYPE
fi

# For coverity, we don't need to actually run tests, just build
if [ -n "$COVERITY" ]; then
  if [[ "$(git branch | grep coverity_scan)" =~ ^.+coverity_scan$ ]]; then
    wget https://scan.coverity.com/download/linux-64 --post-data "token=DW98q3VnP4QKLy4wwLwReQ&project=fluffos%2Ffluffos" -O coverity_tool.tgz
    tar zxvf coverity_tool.tgz
    $PWD/cov-analysis-linux64-*/bin/cov-build --dir cov-int make -j 2
    tar czvf cov.tgz cov-int
    curl --form token=DW98q3VnP4QKLy4wwLwReQ \
         --form email=sunyucong@gmail.com \
         --form file=@cov.tgz \
         --form version="$(git describe --always)" \
         --form description="FluffOS Autobuild" \
         https://scan.coverity.com/builds?project=fluffos%2Ffluffos
    exit 0
  fi
fi

# Otherwise, continue
make -j 2

cd testsuite

# FIXME: currently RELEASE build would report leak on valgrind, thus ignoring it for now.
if [ "$TYPE" = "develop" ]; then
  VALGRIND="valgrind --error-exitcode=255 --suppressions=../valgrind.supp"
else
  VALGRIND="valgrind"
fi

# Run standard test first
$VALGRIND --malloc-fill=0x75 --free-fill=0x73 --track-origins=yes --leak-check=full ../driver etc/config.test -ftest -d
wait $!
if [ $? -ne 0 ]; then
  exit $?
fi
# run special interactive tests
( sleep 30 ; expect telnet_test.expect localhost 4000 ) &
( ../driver etc/config.test -d ) &
wait $!
if [ $? -ne 0 ]; then
  exit $?
fi

if [ -n "$GCOV" ]; then
  cd ..
  pip install cpp-coveralls
  coveralls --exclude packages --exclude thirdparty --exclude testsuite --exclude-pattern '.*.tab.+$' --gcov /usr/bin/gcov-4.8 --gcov-options '\-lp' -r $PWD -b $PWD
fi
