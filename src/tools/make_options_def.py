#!/usr/bin/env python

# This python program accept a list of "#define key value"
# and output a c++ header file

import sys
import re

# NOTE: must use non-greedy match for key part.
DEFINE_RE = re.compile('^#define (.+?) (.*)$')

TPL_HEADER = """
/* Automatically generated by make_options_def.py. */

#ifndef OPTIONS_DEFS_H
#define OPTIONS_DEFS_H

const char* option_defs[] = {
"""

TPL_FOOTER = """
};

#endif
"""

if len(sys.argv) != 2:
  print "Usage: echo "<defines>" | make_options_def.py <output file>"
  sys.exit(1)

result = TPL_HEADER

for line in sys.stdin.readlines():
  m = DEFINE_RE.match(line)
  if not m:
    print "Unexpected line: %s " % line
    os.exit(1)

  key = m.group(1)
  value = m.group(2)

  if key.startswith("_"):
    continue
  key = '"__%s__"' % m.group(1)

  if not value:
    value = '""'
  else:
    if not value.startswith('"'):
	  value = '"%s"' % value

  result = result + '    %s, %s,\n' %  (key, value)

result += TPL_FOOTER

target_file = sys.argv[1]
f = open(target_file, "w")
f.write(result)
f.close()

sys.exit(0)
