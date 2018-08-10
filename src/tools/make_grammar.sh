#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

TOP_SRCDIR="${1}"
shift
SRCDIR="${1}"
shift

OPTIONS_H="${TOP_SRCDIR}/src/base/internal/options_incl.h"
GRAMMAR_Y_PRE="${SRCDIR}"/grammar.y.pre
GRAMMAR_Y=grammar.y

# First step is to run grammar.y.pre through CPP.
"$@" -E -x c++ -undef -P -CC \
  -imacros $OPTIONS_H $GRAMMAR_Y_PRE > $GRAMMAR_Y

# Second step is to run through sed, replacing // #include into #include,
# NOTE that sed -i behave different on linux & BSD, we can't use it.
sed -e 's/\/\/ #include/#include/g' $GRAMMAR_Y > ${GRAMMAR_Y}.tmp
mv ${GRAMMAR_Y}.tmp ${GRAMMAR_Y}
