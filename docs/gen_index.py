#!/usr/bin/env python3

import time
import os
import sys
from typing import Dict, List


def main(basedir, title, version):
    header = f"""---
layout: doc
title: {title}
---
"""

    cats = sorted(os.listdir("./%s" % basedir))

    if 'index.md' in cats:
        cats.remove('index.md')

    all_md_files = [x for x in cats if x.endswith('.md')]

    # if all files are .md files, then we are in a category page directory
    if len(all_md_files) == len(cats):
        with open(f"./{basedir}/index.md", 'w') as f:
            f.write(header)
            for cat in cats:
                f.write(f"* [{cat[:-3]}]({cat[:-3]}.html)" + "\n")
        return

    category_efun_map: Dict[str, List[str]] = {}

    for cat in cats:
        if not '.md' in cat:
            category_efun_map[cat] = os.listdir('./%s/%s' % (basedir, cat))

    with open(f"./{basedir}/index.md", 'w') as f:
        f.write(header)
        for cat in sorted(category_efun_map.keys()):
            f.write(f"## {cat}" + "\n")

            # remove ".md" suffix
            items = sorted([x[:-3] for x in category_efun_map[cat] if x.endswith('.md')])
            for item in items:
                if item != 'index':
                    f.write(f"* [{item}]({cat}/{item}.html)" + "\n")

        # build index.md for subdir
        for cat in sorted(category_efun_map.keys()):
            main(basedir + "/" + cat, cat, version)

if __name__ == '__main__':
    main(*sys.argv[1:])
