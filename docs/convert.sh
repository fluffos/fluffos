#!/bin/bash
set -euo pipefail

TARGET_DIR=${1:-unknown}
VERSION=${2:-unknown}

# Translate man page to text

for dir in $TARGET_DIR/*; do
  for file in $(find $dir -type f -name "*.3") $(find $dir -type f -name "*.4"); do
    echo "[CONVERT] $file"
    sed "1i.c2 ." $file | groff -ww -man -c -Tutf8 -rCR=1 -rIN=4n -rLL=75n | uniq | col -x -b | tail -n +3 | head -n -1 > ${file%.*}.txt
    #rm ${file}
  done
done

# Convert text to md

for dir in $TARGET_DIR/*; do
  for file in $(find $dir -type f -name "*.txt"); do
    TMP=$file.tmp
    TARGET=${file%.*}.md

    cat $file >> $TMP
    rm $file
    sed -r -i 's/^([A-Z][A-Z ]+)$/\n### \1\n/' $TMP
    #sed -r -i 'N;s/\s*\n^(###.+)$/\1/' $TMP

    #sed -r -i "s/\`/'/g" $TARGET

    echo "[Generate] $TARGET"
    echo "" > $TARGET
    echo "---" > $TARGET
    echo "layout: default" >> $TARGET
    echo "title: `basename $dir` / `basename ${file} .txt`" >> $TARGET
    echo "---" >> $TARGET
    #echo "" >> $TARGET
    cat $TMP | uniq >> $TARGET
    rm $TMP

  done
done
