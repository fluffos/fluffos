#!/usr/bin/env python3
"""Report leftover MudOS man-page leftovers in the reference trees.

Usage: python3 docs/lint_manpages.py

Exits 0 always (advisory). CI does not run this yet; use it to batch-fix
headings, SYNOPSYS typos, and `layout: doc` frontmatter.
"""

from __future__ import annotations

import os
import re
import sys

ROOT = os.path.dirname(os.path.realpath(__file__))
TREES = ("efun", "apply")


def main() -> int:
    issues: list[str] = []
    for tree in TREES:
        base = os.path.join(ROOT, tree)
        for dirpath, dirnames, filenames in os.walk(base):
            dirnames[:] = [d for d in dirnames if d != "node_modules"]
            for name in filenames:
                if not name.endswith(".md"):
                    continue
                path = os.path.join(dirpath, name)
                rel = os.path.relpath(path, ROOT)
                with open(path, encoding="utf-8") as f:
                    text = f.read()
                if re.search(r"^layout:\s*doc\s*$", text, re.M):
                    issues.append(f"{rel}: frontmatter layout: doc")
                if re.search(r"SYNOPSYS", text):
                    issues.append(f"{rel}: SYNOPSYS typo")
                if re.search(r"MudOS 3\.1", text):
                    issues.append(f"{rel}: leftover MudOS 3.1 version line")
                if re.search(r"\b[a-z_]+\([34]\)", text) and "SEE ALSO" in text:
                    issues.append(f"{rel}: man-page SEE ALSO (name(3)/name(4))")
    print(f"{len(issues)} advisory finding(s)")
    for line in issues[:80]:
        print(line)
    if len(issues) > 80:
        print(f"... {len(issues) - 80} more")
    return 0


if __name__ == "__main__":
    sys.exit(main())
