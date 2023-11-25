#!/usr/bin/env python3

# list all files under EFUN/APPLY

import os

def list_md_files(directory):
    md_files = {}
    md_files[directory] = {}
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file == 'index.md':
                continue
            if file.endswith('.md'):
                md_files[directory][file[:-3]] = os.path.join(root, file)[:-3] + ".html"

    return md_files

APPLYS = list_md_files("apply")
EFUNS = list_md_files("efun")

print(APPLYS)
print(EFUNS)