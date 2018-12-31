#!/bin/bash

# Download a tag from git repo

CLONE_DIR=/tmp/fluffos

git clone https://github.com/fluffos/fluffos.git $CLONE_DIR --depth=5
cd $CLONE_DIR
TAG=$(git rev-parse --short --verify HEAD)
echo "Using tag $TAG..."
cd -

# Generate EFUN doc

TARGET_DIR=doc/efun
rm -rf $TARGET_DIR
mkdir $TARGET_DIR

cp -rf $CLONE_DIR/doc/efuns/. $TARGET_DIR/

./convert.sh $TARGET_DIR $TAG
./gen_index.py $TARGET_DIR EFUN $TAG > $TARGET_DIR/index.md

# Generate APPLY doc

TARGET_DIR=doc/apply
rm -rf $TARGET_DIR
mkdir $TARGET_DIR

cp -rf $CLONE_DIR/doc/applies/* $TARGET_DIR/

./convert.sh $TARGET_DIR $TAG
./gen_index.py $TARGET_DIR Applies $TAG > $TARGET_DIR/index.md

# Copy rest of docs
for topic in concepts driver lpc; do
  TARGET_DIR=doc/$topic
  rm -rf $TARGET_DIR
  mkdir $TARGET_DIR
  cp -rf $CLONE_DIR/doc/$topic doc/
  ./convert.sh $TARGET_DIR $TAG
  ./gen_index.py $TARGET_DIR $topic $TAG > $TARGET_DIR/index.md
done

# some other stuff

# Cleanup
echo "done"
rm -rf $CLONE_DIR
