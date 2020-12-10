#!/usr/bin/env python3

import time
import os
import sys
from typing import Dict, List


def main(basedir, title, version):
  category_efun_map: Dict[str, List[str]] = {}

  cats = sorted(os.listdir("./%s" % basedir))
  for cat in cats:
    if not '.md' in cat:
      category_efun_map[cat] = os.listdir('./%s/%s' % (basedir, cat))

  print(f"""---
layout: default
title: {title}
---

""")

  for cat in sorted(category_efun_map.keys()):
    print("### %s\n" % cat)
    print("<div class='container'>")
    i = 0

    # remove ".md" suffix
    items = sorted([x[:-3] for x in category_efun_map[cat] if x.endswith('.md')])

    while i < len(items):
      print("<div class='row'>")
      for item in items[i: i + 4]:
        print("<div class='col-sm-3'>")
        print(f"<div><a href='{cat}/{item}.html'>{item}</a></div>")
        print("</div>")
      i = i + 4
      # round up
      if i > len(items):
        for x in range(i - len(items)):
          print("<div>&nbsp;</div>")
      print("</div>")
    print("</div>\n")

  print(f"""

This page is auto generated on {time.strftime("%Y-%m-%d %H:%M:%S %Z", time.localtime())} for [{version}](https://github.com/fluffos/fluffos/tree/{version}).

""")


if __name__ == '__main__':
  main(*sys.argv[1:])
