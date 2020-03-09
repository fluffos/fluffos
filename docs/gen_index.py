#!/usr/bin/python

import time
import os
import sys

def main(basedir, title, version):

  category_efun_map = {}

  cats = sorted(os.listdir("./%s" % basedir))
  for cat in cats:
    if not '.md' in cat:
      category_efun_map[cat] = os.listdir('./%s/%s' % (basedir, cat))

  print """---
layout: default
title: %s
---

Auto generated on %s for [%s](https://github.com/fluffos/fluffos/tree/%s).

""" % (title, time.strftime("%Y-%m-%d %H:%M:%S %Z", time.localtime()), version, version)

  for cat in sorted(category_efun_map.iterkeys()):
    print "### %s\n" % cat
    print "<table class='table table-condensed'>"
    i = 0

    # remove ".md" suffix
    items = sorted([ x[:-3] for x in category_efun_map[cat] if x.endswith('.md')])

    while i < len(items):
      print "<tr>"
      for item in items[i: i+5]:
        print "<td>"
        print "<a href='%s'>%s</a>" % ("%s/%s.html" % (cat, item), item)
        print "</td>"
      i = i + 5
      # round up
      if i > len(items):
        for x in xrange(i - len(items)):
          print "<td></td>"
      print "</tr>"
    print "</table>\n"

if __name__ == '__main__':
  main(*sys.argv[1:])
