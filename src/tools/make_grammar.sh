#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

OPTIONS_H=base/internal/options_incl.h
GRAMMAR_Y_PRE=vm/internal/compiler/grammar.y.pre
GRAMMAR_Y=vm/internal/compiler/grammar.autogen.y

# First step is to run grammar.y.pre through CPP.
#"$@" -E -x c++ -undef -P -CC -imacros $OPTIONS_H $GRAMMAR_Y_PRE > $GRAMMAR_Y

# Second step is to run through sed, replacing $include into #include
#sed -e 's/\/\/ #include/#include/g' -i $GRAMMAR_Y


# First step is to run grammar.y.pre through CPP.
# Second step is to run through sed, replacing $include into #include
"$@" -E -x c++ -undef -P -CC -imacros $OPTIONS_H $GRAMMAR_Y_PRE | sed -e 's/\/\/ #include/#include/g' > $GRAMMAR_Y

echo $GRAMMAR_Y
