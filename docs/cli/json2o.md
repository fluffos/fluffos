---
layout: doc
title: cli / json2o
---

# cli / json2o

`json2o` is a tool to convert JSON to LPC save file format, it only accepts JSON in the format produced by `o2json`.

## Usage

```bash
./json2o json_file o_file
```

## Note

if `json_file` is `-`, this program will read from stdin, and if `o_file` is `-`, this program will write to stdout,
you can then pipe the output to other programs.

## JSON format

see [o2json](/cli/o2json.html) for details.


