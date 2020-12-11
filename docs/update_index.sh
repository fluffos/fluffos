#!/bin/bash

TAG=$(git describe --tags --always --dirty)
echo "Using commit $TAG..."

# Generate EFUN doc

TARGET_DIR=./efun
./gen_index.py $TARGET_DIR EFUN $TAG > $TARGET_DIR/index.md

# Generate APPLY doc

TARGET_DIR=./apply
./gen_index.py $TARGET_DIR APPLY $TAG > $TARGET_DIR/index.md

TARGET_DIR=./stdlib
./gen_index.py $TARGET_DIR STDLIB $TAG > $TARGET_DIR/index.md

# Generate zh-CN docs

TARGET_DIR=./zh-CN
./gen_index.py $TARGET_DIR zh-CN $TAG > $TARGET_DIR/index.md

TARGET_DIR=./zh-CN/apply
./gen_index.py $TARGET_DIR APPLY $TAG > $TARGET_DIR/index.md

# Copy rest of docs
for topic in concepts driver lpc; do
  TARGET_DIR=./$topic
  ./gen_index.py $TARGET_DIR $topic $TAG > $TARGET_DIR/index.md
done
