---
layout: doc
title: cli / o2json
---

# cli / o2json

`o2json` is a tool to convert LPC save file to JSON, combined with `json2o`, it can enable you to manipulate LPC save file using JSON tools.

## Usage

```bash
./o2json [flags] o_file json_file
```

## Note

if `o_file` is `-`, this program will read from stdin, and if `json_file` is `-`, this program will write to stdout,
you can then pipe the output to other programs.

## Flags

| Flag     | Description                                                |
|----------|------------------------------------------------------------|
| `-prtty` | Print JSON with newline and indent                         |
| `-ascii` | Print all non-ascii characters in `\uXXXX` escaped format. |

## JSON format

The JSON output is an object with the following fields:
```json
{
  "program_name": "program_name",
  "variables": [ variable_object ]
}
```

The `variable_object` is an object with the following fields:
```json
{
  "name": "variable_name",
  "value": svalue
}
```

The `svalue` is a object with the following fields:
```json
{
  "type": type,
  "value": value
}
``` 

The `type` is a string, it can be one of the following values:
`"int"`, `"float"`, `"string"`, `"array"`, `"mapping"`, `"buffer"`

The `value` is either a number, a string, or an array of `svalue` objects.