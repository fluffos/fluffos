#!/usr/bin/env python3
"""Generate stub docs under efun/general/ for efuns that have no doc page.

Usage: add_missing_efuns.py path/to/keywords.json

keywords.json is produced by the generate_keywords tool from a compiled
driver (run it in your build directory). Run this script from docs/.
"""

import json
import os
import sys


def generate_signature(efun):
    args = [" | ".join(arg_type) for arg_type in efun["args_types"]]
    if efun["max_args"] == -1:
        args.append("...")
    return f"""{efun["returns"]} {efun["name"]}({", ".join(args)})"""


def generate_doc(efun):
    return f"""---
title: general / {efun["name"]}
---
# {efun["name"]}

### NAME

    {efun["name"]}

### SYNOPSIS

    {generate_signature(efun)}

### DESCRIPTION

    TBW

"""


def main():
    if len(sys.argv) != 2 or not os.path.isfile(sys.argv[1]):
        sys.exit(__doc__.strip().splitlines()[2])  # the Usage line

    documented = set()
    for root, dirs, files in os.walk("./efun"):
        for file in files:
            if file.endswith(".md") and file != "index.md":
                documented.add(file[:-3])
    print(f"EFUNS with docs: {len(documented)}")

    keywords = json.load(open(sys.argv[1]))
    all_efuns = {keyword["name"]: keyword for keyword in keywords}
    print(f"ALL EFUNS: {len(all_efuns)}")

    for name, efun in all_efuns.items():
        if name.startswith("_") or name in documented:
            continue
        print(f"Generating doc for {name}")
        with open(f"./efun/general/{name}.md", "w") as f:
            f.write(generate_doc(efun))


if __name__ == "__main__":
    main()
