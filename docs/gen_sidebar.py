#!/usr/bin/env python3
"""Generate sidebars.generated.json for the reference doc trees (efun/, apply/, ...).

Usage: gen_sidebar.py [--check]

Replaces the old gen_index.py, which wrote per-directory index.md link pages.
Navigation is now the Docusaurus sidebar itself: this script walks each
reference tree and emits a fully-expandable category tree per directory.
Category landing pages are Docusaurus `generated-index` pages (auto-generated
card grids), so no index.md files are written or needed in these trees.

Curated presentation lives in sidebar_meta.json, keyed by directory (or page)
path relative to docs/:

    "efun/arrays":  { "label": "Arrays", "description": "..." }
    "driver":       { "label": "...", "description": "...",
                      "order": ["config", "wasm", ...],
                      "labels": { "config": "Runtime Configuration" } }

Per-directory meta keys:
    label        sidebar label for the category (default: directory name)
    description  shown on the generated-index landing page
    order        page/subdir names listed first, in this order; anything not
                 listed follows alphabetically (subdirectories, then pages)
    labels       per-page sidebar label overrides (default: file name)

sidebars.ts imports sidebars.generated.json and splices each tree into the
sidebar skeleton. Run this script after adding/removing/moving any page in
the trees below; CI verifies freshness with --check.
"""

import json
import os
import sys

DOCS_ROOT = os.path.dirname(os.path.realpath(__file__))
META_FILE = os.path.join(DOCS_ROOT, "sidebar_meta.json")
OUT_FILE = os.path.join(DOCS_ROOT, "sidebars.generated.json")

# The doc trees this script owns. lpc/ is NOT here: its index pages are
# hand-written and its sidebar is hand-authored in sidebars.ts. The Chinese
# corpus lives in i18n/zh-CN/ (Docusaurus i18n) and shares this sidebar;
# its category labels are translated in
# i18n/zh-CN/docusaurus-plugin-content-docs/current.json.
TREES = ["efun", "apply", "stdlib", "concepts", "driver", "cli"]

# Directory entries that are never documentation content.
SKIP_DIRS = {"node_modules"}


def load_meta():
    with open(META_FILE, encoding="utf-8") as f:
        return json.load(f)


def list_entries(path):
    """Return (md_files, subdirs) for a directory, sorted and filtered.

    md_files are page names without the .md/.mdx extension (index excluded);
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
        else:
            for ext in (".md", ".mdx"):
                if entry.endswith(ext) and entry[: -len(ext)] != "index":
                    files.append(entry[: -len(ext)])
                    break
    return files, subdirs


def apply_order(names, order):
    """Names from `order` first (in that order), the rest keep sorted order."""
    if not order:
        return names
    ordered = [n for n in order if n in names]
    return ordered + [n for n in names if n not in ordered]


def build_category(meta, rel):
    """Build the sidebar category item for directory `rel` (docs-relative)."""
    path = os.path.join(DOCS_ROOT, rel)
    files, subdirs = list_entries(path)
    dir_meta = meta.get(rel, {})
    label = dir_meta.get("label", os.path.basename(rel))
    labels = dir_meta.get("labels", {})
    order = dir_meta.get("order", [])

    # Subdirectories first, then loose pages — with `order` pulling any
    # explicitly-listed names (of either kind) to the front.
    items = []
    for name in apply_order(subdirs + files, order):
        if name in subdirs:
            items.append(build_category(meta, f"{rel}/{name}"))
        else:
            items.append(
                {
                    "type": "doc",
                    "id": f"{rel}/{name}",
                    # Unique translation key (labels repeat: e.g. hash exists
                    # in both efun/crypto and efun/strings).
                    "key": f"{rel}/{name}",
                    "label": labels.get(name, name),
                }
            )

    # A stable, unique translation key: labels repeat across trees (both
    # efun/ and stdlib/ have "Arrays"), which would collide in i18n
    # translation files (sidebar.docs.category.<key>).
    category = {"type": "category", "key": rel, "label": label}
    if os.path.exists(os.path.join(path, "index.md")):
        # A hand-written landing page wins over the generated card grid.
        category["link"] = {"type": "doc", "id": f"{rel}/index"}
    else:
        link = {
            "type": "generated-index",
            "title": label,
            "slug": f"/{rel}/",
        }
        if "description" in dir_meta:
            link["description"] = dir_meta["description"]
        category["link"] = link
    category["items"] = items
    return category


def generate():
    meta = load_meta()
    return {tree: build_category(meta, tree) for tree in TREES}


def render(sidebars):
    return json.dumps(sidebars, indent=2, ensure_ascii=False) + "\n"


def main(argv):
    output = render(generate())
    if "--check" in argv:
        try:
            with open(OUT_FILE, encoding="utf-8") as f:
                current = f.read()
        except FileNotFoundError:
            current = ""
        if current != output:
            sys.exit(
                "sidebars.generated.json is stale; run docs/gen_sidebar.py "
                "and commit the result"
            )
        return
    with open(OUT_FILE, "w", encoding="utf-8") as f:
        f.write(output)


if __name__ == "__main__":
    main(sys.argv[1:])
