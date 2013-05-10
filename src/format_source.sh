#!/bin/sh

# astyle option file
# http://astyle.sourceforge.net/astyle.html#_Options

ASTYLE="astyle \
--mode=c \
--style=kr \
--indent=spaces=2 \
--indent-switches \
--pad-oper \
--pad-header \
--unpad-paren \
--keep-one-line-blocks \
--keep-one-line-statements \
--convert-tabs \
--align-pointer=name \
--indent-col1-comments \
--min-conditional-indent=0 \
--lineend=linux \
"

for f in `find . -regex ".*\.[ch]$" | grep -v "compat" | grep -v "testsuite" | grep -v "_spec"`; do
  if [ -s $f ]; then $ASTYLE $f; fi
done
