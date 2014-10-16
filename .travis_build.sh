#!/bin/bash
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get update -qq
sudo apt-get install -qq autoconf
sudo apt-get install -qq gcc-4.8 g++-4.8
sudo apt-get install -qq bison

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 100 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8
sudo update-alternatives --auto gcc
sudo update-alternatives --query gcc

if [ "$BUILD" = "i386" ]; then
  sudo apt-get remove libevent-dev libevent-* libssl-dev

  sudo apt-get install g++-multilib g++-4.8-multilib
  sudo apt-get --no-install-recommends install valgrind:i386

  sudo apt-get install libevent-2.0-5:i386
  sudo apt-get install libevent-dev:i386
  sudo apt-get --no-install-recommends install libz-dev:i386
else
  sudo apt-get install valgrind
  sudo apt-get install libevent-dev libmysqlclient-dev libsqlite3-dev libpq-dev libz-dev libssl-dev libpcre3-dev
fi

# stop on first error down below
set -e
cd src
./autogen.sh
cp local_options.$CONFIG local_options
if [ -n "$GCOV" ]; then
  ./build.FluffOS $TYPE --enable-gcov=yes
else
  ./build.FluffOS $TYPE

make -j 2
cd testsuite

if [ -n "$GCOV" ]; then
  # run in gcov mode and submit the result
  ../driver etc/config.test -ftest -d
  sudo pip install cpp-coveralls
  coveralls --exclude thirdparty --gcov-options '\-lp'
else
  valgrind --malloc-fill=0x75 --free-fill=0x73 --track-origins=yes --leak-check=full ../driver etc/config.test -ftest -d
