#!/bin/sh

# astyle option file
# http://astyle.sourceforge.net/astyle.html#_Options

#ASTYLE="astyle \
#--mode=c \
#--style=kr \
#--indent=spaces=2 \
#--indent-switches \
#--pad-oper \
#--pad-header \
#--unpad-paren \
#--keep-one-line-blocks \
#--keep-one-line-statements \
#--convert-tabs \
#--align-pointer=name \
#--indent-col1-comments \
#--min-conditional-indent=0 \
#--lineend=linux \
#"

for f in `find . -regextype posix-extended -regex ".*\.(cc|h)$" | grep -v "thirdparty" | grep -v "compat" | grep -v "testsuite" `; do
  if [ -s $f ]; then $1 -i --style="{BasedOnStyle: google, ColumnLimit: 100}" $f; fi
done
