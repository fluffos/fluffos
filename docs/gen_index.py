#!/usr/bin/env python3
"""Regenerate index.md files for the reference doc trees (efun/, apply/, ...).

Usage: gen_index.py DIR [TITLE]

Writes DIR/index.md listing every doc page in DIR, grouped by subdirectory,
then recurses into each subdirectory. Links are extension-less relative URLs
(Docusaurus resolves `arrays/allocate` against the index page's route).

Driven by update_index.sh; do NOT run it on hand-written index pages
(e.g. lpc/index.md).
"""

import os
import sys

# Directory entries that are never documentation content.
SKIP_DIRS = {"node_modules"}


def list_entries(path):
    """Return (md_files, subdirs) for a directory, sorted and filtered.

    md_files are page names without the .md extension (index excluded);
    subdirs are documentation subdirectories.
    """
    files = []
    subdirs = []
    for entry in sorted(os.listdir(path)):
        if entry.startswith(".") or entry in SKIP_DIRS:
            continue
        full = os.path.join(path, entry)
        if os.path.isdir(full):
            subdirs.append(entry)
        elif entry.endswith(".md") and entry != "index.md":
            files.append(entry[:-3])
    return files, subdirs


def write_index(path, title):
    files, subdirs = list_entries(path)

    lines = ["---", f"title: {title}", "---"]

    # Loose pages directly in this directory.
    for name in files:
        lines.append(f"* [{name}]({name})")

    # One section per subdirectory.
    for sub in subdirs:
        lines.append(f"## {sub}")
        sub_files, sub_subdirs = list_entries(os.path.join(path, sub))
        for nested in sub_subdirs:
            lines.append(f"* [{nested}]({sub}/{nested}/)")
        for name in sub_files:
            lines.append(f"* [{name}]({sub}/{name})")

    with open(os.path.join(path, "index.md"), "w") as f:
        f.write("\n".join(lines) + "\n")

    for sub in subdirs:
        write_index(os.path.join(path, sub), sub)


def main(argv):
    if len(argv) < 1 or not os.path.isdir(argv[0]):
        sys.exit(__doc__.strip().splitlines()[2])  # the Usage line
    path = argv[0].rstrip("/")
    if os.path.realpath(path) == os.path.dirname(os.path.realpath(__file__)):
        sys.exit("refusing to run on the docs root; pass a doc tree like efun/ or apply/")
    title = argv[1] if len(argv) > 1 else os.path.basename(path)
    write_index(path, title)


if __name__ == "__main__":
    main(sys.argv[1:])
