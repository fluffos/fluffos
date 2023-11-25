#!/usr/bin/env python3

# add a "# Name" for all md files

import os


def add_title(title, lines):
    # skip if the file already have a title
    for line in lines:
        if line.startswith("# "):
            return None

    # find the first line after frontmatter and add title
    try:
        first_idx = lines.index("---\n")
        second_idx = lines.index("---\n", first_idx + 1)

        if second_idx - first_idx > 1:
            lines.insert(second_idx + 1, "# " + title + "\n")
            return lines
    except ValueError:
        return None

    return None


def main():
    for root, dirs, files in os.walk("."):
        for file in files:
            if file.endswith(".md"):
                if file == "index.md":
                    continue
                with open(os.path.join(root, file), "r+") as f:
                    original_lines = f.readlines()
                    new_lines = add_title(file[:-3], original_lines)
                    if new_lines and len(new_lines) > 0:
                        print("modifying " + file + "...")
                        f.seek(0)
                        f.truncate()
                        f.writelines(new_lines)


if __name__ == "__main__":
    main()
