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

""" % (title)

  for cat in sorted(category_efun_map.iterkeys()):
    print "### %s\n" % cat
    print "<div class='container'>"
    i = 0

    # remove ".md" suffix
    items = sorted([ x[:-3] for x in category_efun_map[cat] if x.endswith('.md')])

    while i < len(items):
      print "<div class='row'>"
      for item in items[i: i+4]:
        print "<div class='col-sm-3'>"
        print "<div><a href='%s'>%s</a></div>" % ("%s/%s.html" % (cat, item), item)
        print "</div>"
      i = i + 4 
      # round up
      if i > len(items):
        for x in xrange(i - len(items)):
          print "<div>&nbsp;</div>"
      print "</div>"
    print "</div>\n"

  print """

This page is auto generated on %s for [%s](https://github.com/fluffos/fluffos/tree/%s).

""" % (time.strftime("%Y-%m-%d %H:%M:%S %Z", time.localtime()), version, version)

if __name__ == '__main__':
  main(*sys.argv[1:])
