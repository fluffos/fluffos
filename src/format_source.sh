#!/bin/sh

for f in $(find . -type f -name "*.cc" | grep -v "thirdparty" | grep -v "compat" | grep -v "testsuite" ); do
  if [ -s "$f" ]; then $1 -i $f; fi
done

for f in $(find . -type f -name "*.h" | grep -v "thirdparty" | grep -v "compat" | grep -v "testsuite" ); do
  if [ -s "$f" ]; then $1 -i $f; fi
done
