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

$CC -v
$CXX -v

cd /tmp
mkdir .build
cd .build
cmake -DCMAKE_BUILD_TYPE=RELEASE /usr/src/gtest/
make
sudo mv libg* /usr/lib/

}

D=`pwd`
# do setup
setup

cd $D

# stop on first error down below
set -eo pipefail

# testing part
# cd src            <= configure base dir now top dir
# ./autogen.sh      <= shouldn't be neccessary any more

############# OLD
#cp local_options local_options.default
#cp local_options.$CONFIG local_options

#if [ -n "$GCOV" ]; then
#  ./build.FluffOS $TYPE --enable-gcov=yes
#else
#  ./build.FluffOS $TYPE
#fi
#############

############# NEW
# make build directory and change into it
mkdir build
cd build
# configure with additional warnings enabled
../configure --enable-devel
#############

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
make -j 2 V=1

# Run standard test first
cd ../src/testsuite
../../build/src/driver etc/config.test -ftest -d
wait $!

if [ $? -ne 0 ]; then
  exit $?
fi
# run special interactive tests
( sleep 30 ; expect telnet_test.expect localhost 4000 ) &
( ../../build/src/driver etc/config.test -d ) &
wait $!
if [ $? -ne 0 ]; then
  exit $?
fi

if [ -n "$GCOV" ]; then
  cd ..
  pip install --user cpp-coveralls
  coveralls --exclude thirdparty --exclude testsuite --exclude-pattern '.*.autogen.+$' --gcov /usr/bin/gcov-4.8 --gcov-options '\-lp' -r $PWD -b $PWD
fi
