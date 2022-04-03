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
    print(f"""
<h2 class="border-bottom py-3 text-uppercase">
  <img src="https://cdnjs.cloudflare.com/ajax/libs/octicons/8.5.0/svg/bookmark.svg"><a class='text-secondary' name="{cat}" href="#{cat}">{cat}</a>
</h2>
""")
    i = 0

    # remove ".md" suffix
    items = sorted([x[:-3] for x in category_efun_map[cat] if x.endswith('.md')])

    print("<div class='row'>")
    while i < len(items):
      for item in items[i: i + 4]:
        print("<div class='col-sm-4 col-md-3 col-lg-3 col-xl-2'>")
        print(f"<div><a href='{cat}/{item}.html'>{item}</a></div>")
        print("</div>")
      i = i + 4
      # round up
      #if i > len(items):
      #  for x in range(i - len(items)):
      #    print("<div>&nbsp;</div>")
    print("</div>")

  print(f"""
<div class="alert alert-info my-4" role="alert">
    <img src="https://cdnjs.cloudflare.com/ajax/libs/octicons/8.5.0/svg/info.svg">
    This page is auto generated on {time.strftime("%Y-%m-%d %H:%M:%S %Z", time.localtime())} for {version}.</a>
</div>
""")


if __name__ == '__main__':
  main(*sys.argv[1:])
