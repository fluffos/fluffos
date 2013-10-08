#!/bin/sh
sudo apt-get install -qq bison

if [ "$BUILD" = "i386" ]; then
  sudo apt-get remove libevent-dev libevent* libssl-dev

  sudo apt-get install g++-multilib
  sudo apt-get --no-install-recommends install valgrind:i386

  sudo apt-get install libevent-2.0-5:i386
  sudo apt-get install libevent-dev:i386
  sudo apt-get --no-install-recommends install libz-dev:i386
else
  sudo apt-get install valgrind
  sudo apt-get install libevent-dev libmysqlclient-dev libsqlite3-dev libpq-dev libz-dev libssl-dev libpcre3-dev
fi

cd src && cp local_options.$CONFIG local_options && ./build.FluffOS $TYPE && make && cd testsuite && valgrind --malloc-fill=0x75 --free-fill=0x73 --track-origins=yes --leak-check=full ../driver etc/config.test -ftest -d
