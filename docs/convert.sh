#!/bin/bash
set -euo pipefail

TARGET_DIR=${1:-unknown}
VERSION=${2:-unknown}

# Translate man page to text

for dir in $TARGET_DIR/*; do
  for file in $(find $dir -type f -name "*.3") $(find $dir -type f -name "*.4"); do
    echo "[CONVERT] $file"
    groff -mandoc -c -Tutf8 $file | col -x -b | tail -n +2 | head -n -1 > ${file%.*}
    rm ${file}
  done
done

# Convert text to md

for dir in $TARGET_DIR/*; do
  for file in $(find $dir -type f); do
    TMP=$file.tmp
    TARGET=${file%.*}.md
    echo "---" > $TMP
    echo "layout: default" >> $TMP
    echo "title: `basename $dir` / `basename ${file}`" >> $TMP
    echo "---" >> $TMP
    echo "" >> $TMP
    echo "" >> $TMP

    cat $file >> $TMP
    rm $file

    mv $file.tmp $TARGET
    echo "[Generate] $TARGET"

    sed -r -i 's/^([A-Z ]+)$/### \1/' $TARGET
    sed -r -i 's/^ +/    /' $TARGET
    sed -r -i "s/\`/'/g" $TARGET
  done
done
