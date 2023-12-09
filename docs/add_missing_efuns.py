#!/usr/bin/env python3

# find all existing efuns through keywords.json, then generate doc under efun/general/

import os
import json

EFUNS_DOCS = set()


def generate_signature(efun):
    args = [" | ".join(arg_type) for arg_type in efun["args_types"]]
    if efun["max_args"] == -1:
        args.append("...")
    return f"""{efun["returns"]} {efun["name"]}({", ".join(args)})"""


def generate_doc(efun):
    return f"""---
layout: doc
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
    for root, dirs, files in os.walk("./efun"):
        for file in files:
            if file.endswith(".md"):
                if file == "index.md":
                    continue
                EFUNS_DOCS.add(file[:-3])
    print("EFUNS with docs: " + str(len(EFUNS_DOCS)))

    keywords = json.load(open("./keywords.json"))
    all_efuns = {keyword["name"]: keyword for keyword in keywords}
    print("ALL EFUNS: " + str(len(all_efuns)))

    for efun in all_efuns:
        if efun.startswith("_"):
            continue
        if efun in EFUNS_DOCS:
            continue
        print(f"Generating doc for {efun}")
        content = generate_doc(all_efuns[efun])
        with open(f"./efun/general/{efun}.md", "w") as f:
            f.write(content)


if __name__ == "__main__":
    main()
