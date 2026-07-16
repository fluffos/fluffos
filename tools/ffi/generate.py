#!/usr/bin/env python3
"""Generate LPC bindings for a C library from its header.

    tools/ffi/generate.py <header.h> --lib <libpath> --out <basename>
                          [--string-convenience] [--emit-json]

Emits, next to <basename>:
  <basename>.lpc          -- one LPC wrapper per exported C function.
  <basename>_structs.h    -- ffi_struct_layout arrays + field offsets.

Design & rationale: docs/driver/ffi.md. Key rule: a C char*/data
pointer becomes an LPC `buffer` (never `string`) -- LPC strings are
UTF-8-native and the byte boundary must be explicit. --string-convenience
additionally emits <name>_s(string ...) overloads that string_encode()
to a NUL-terminated UTF-8 buffer for the common case.

This is a deliberately small, tolerant C-subset parser: it handles
`extern`-style function prototypes and `struct { ... }` bodies over the
scalar/pointer types the FFI package supports, and reports+skips anything
it cannot map (function pointers, unions, bitfields, varargs) rather than
mis-binding it.
"""
import argparse
import json
import re
import sys

# C type spelling -> (FFI_* code, LPC type, is_pointer). Longest/most
# specific spellings first; the matcher normalizes whitespace.
TYPE_MAP = [
    ("void", ("FFI_VOID", "void", False)),
    ("_Bool", ("FFI_UINT8", "int", False)),
    ("bool", ("FFI_UINT8", "int", False)),
    ("unsigned char", ("FFI_UINT8", "int", False)),
    ("signed char", ("FFI_INT8", "int", False)),
    ("char", ("FFI_INT8", "int", False)),
    ("unsigned short int", ("FFI_UINT16", "int", False)),
    ("unsigned short", ("FFI_UINT16", "int", False)),
    ("short int", ("FFI_INT16", "int", False)),
    ("short", ("FFI_INT16", "int", False)),
    ("unsigned long long", ("FFI_UINT64", "int", False)),
    ("long long int", ("FFI_INT64", "int", False)),
    ("long long", ("FFI_INT64", "int", False)),
    ("unsigned long int", ("FFI_UINT64", "int", False)),
    ("unsigned long", ("FFI_UINT64", "int", False)),
    ("long int", ("FFI_INT64", "int", False)),
    ("long", ("FFI_INT64", "int", False)),
    ("unsigned int", ("FFI_UINT32", "int", False)),
    ("unsigned", ("FFI_UINT32", "int", False)),
    ("int", ("FFI_INT32", "int", False)),
    ("size_t", ("FFI_UINT64", "int", False)),
    ("ssize_t", ("FFI_INT64", "int", False)),
    ("int8_t", ("FFI_INT8", "int", False)),
    ("uint8_t", ("FFI_UINT8", "int", False)),
    ("int16_t", ("FFI_INT16", "int", False)),
    ("uint16_t", ("FFI_UINT16", "int", False)),
    ("int32_t", ("FFI_INT32", "int", False)),
    ("uint32_t", ("FFI_UINT32", "int", False)),
    ("int64_t", ("FFI_INT64", "int", False)),
    ("uint64_t", ("FFI_UINT64", "int", False)),
    ("double", ("FFI_DOUBLE", "float", False)),
    ("float", ("FFI_FLOAT", "float", False)),
]
TYPE_LOOKUP = dict(TYPE_MAP)


class UnsupportedType(Exception):
    pass


def map_type(spelling):
    """C type spelling -> (ffi_code, lpc_type, is_char_ptr). Raises
    UnsupportedType for anything the FFI package cannot marshal."""
    s = re.sub(r"\s+", " ", spelling).strip()
    s = s.replace("const ", "").strip()
    if "(" in s or ")" in s:
        raise UnsupportedType(spelling)  # function pointer
    if "*" in s:
        base = s.replace("*", " ").strip()
        is_char = base.replace("const", "").strip() in ("char", "unsigned char", "signed char")
        return ("FFI_POINTER", "buffer", is_char)
    if s in TYPE_LOOKUP:
        return TYPE_LOOKUP[s] + (False,)
    raise UnsupportedType(spelling)


# One prototype:  RET NAME ( ARGS ) ;   (return type may have '*'s)
PROTO_RE = re.compile(
    r"(?:extern\s+)?([A-Za-z_][\w\s\*]*?)\b([A-Za-z_]\w*)\s*\(([^;{]*)\)\s*;", re.MULTILINE)
STRUCT_RE = re.compile(
    r"(?:typedef\s+)?struct\s+([A-Za-z_]\w*)\s*\{([^}]*)\}\s*;", re.MULTILINE)


def strip_comments(src):
    src = re.sub(r"/\*.*?\*/", " ", src, flags=re.DOTALL)
    src = re.sub(r"//[^\n]*", " ", src)
    return src


def split_args(argstr):
    argstr = argstr.strip()
    if argstr in ("", "void"):
        return []
    if "..." in argstr:
        raise UnsupportedType("varargs")
    return [a.strip() for a in argstr.split(",")]


def arg_type_spelling(arg):
    """A parameter declaration -> its type spelling (drop the name)."""
    a = arg.strip()
    m = re.match(r"^(.*?)([A-Za-z_]\w*)\s*$", a)
    if m and m.group(1).strip() and not m.group(1).strip().endswith("*") is False:
        pass
    # If the last token is a plain identifier AND there's a type before it,
    # treat it as the parameter name and drop it. "int", "char *" have none.
    if m:
        head = m.group(1).strip()
        if head and head not in ("unsigned", "signed", "const", "struct", "long", "short"):
            return head
    return a  # unnamed parameter (just a type)


def parse_functions(src):
    funcs, skipped = [], []
    for m in PROTO_RE.finditer(src):
        ret_spelling, name, argstr = m.group(1).strip(), m.group(2), m.group(3)
        if name in ("if", "for", "while", "switch", "return", "sizeof"):
            continue
        try:
            ret = map_type(ret_spelling)
            args = [map_type(arg_type_spelling(a)) for a in split_args(argstr)]
        except UnsupportedType as e:
            skipped.append((name, str(e)))
            continue
        funcs.append({"name": name, "ret": ret, "args": args})
    return funcs, skipped


def parse_structs(src):
    structs, skipped = [], []
    for m in STRUCT_RE.finditer(src):
        name, body = m.group(1), m.group(2)
        fields = []
        ok = True
        for decl in body.split(";"):
            decl = decl.strip()
            if not decl:
                continue
            fm = re.match(r"^(.*?)([A-Za-z_]\w*)$", decl)
            if not fm:
                ok = False
                break
            try:
                code = map_type(fm.group(1).strip())[0]
            except UnsupportedType:
                ok = False
                break
            fields.append({"name": fm.group(2), "code": code})
        if ok and fields:
            structs.append({"name": name, "fields": fields})
        else:
            skipped.append(name)
    return structs, skipped


def emit_lpc(funcs, libpath, string_convenience):
    lines = [
        "// GENERATED by tools/ffi/generate.py -- do not edit by hand.",
        "// LPC bindings for a native library via package_ffi. Pointer/data",
        "// arguments are `buffer` (LPC strings are UTF-8-native; encode",
        "// explicitly). See docs/driver/ffi.md.",
        "#include <ffi.h>",
        "",
        'private nosave string LIB_PATH = "%s";' % libpath,
        "private nosave int _lib = 0;",
        "private nosave mapping _h = ([ ]);",
        "",
        "private int _lib_handle() {",
        "    if (!_lib) _lib = ffi_load(LIB_PATH);",
        "    return _lib;",
        "}",
        "",
    ]
    for fn in funcs:
        name = fn["name"]
        arg_codes = ", ".join(a[0] for a in fn["args"])
        params = ", ".join("%s a%d" % (a[1], i) for i, a in enumerate(fn["args"]))
        argvals = ", ".join("a%d" % i for i in range(len(fn["args"])))
        rettype = fn["ret"][1]
        lines.append("private int _h_%s() {" % name)
        lines.append('    if (undefinedp(_h["%s"]))' % name)
        lines.append('        _h["%s"] = ffi_prepare(_lib_handle(), "%s", %s, ({ %s }));'
                     % (name, name, fn["ret"][0], arg_codes))
        lines.append('    return _h["%s"];' % name)
        lines.append("}")
        ret_kw = "" if rettype == "void" else "return "
        lines.append("%s %s(%s) {" % (rettype, name, params))
        lines.append("    %sffi_call(_h_%s(), ({ %s }));" % (ret_kw, name, argvals))
        lines.append("}")
        # Optional string-convenience overload for char* params.
        if string_convenience:
            char_ptr = [i for i, a in enumerate(fn["args"]) if a[2]]
            if char_ptr:
                sparams = ", ".join(
                    ("string a%d" % i) if i in char_ptr else ("%s a%d" % (a[1], i))
                    for i, a in enumerate(fn["args"]))
                sargs = ", ".join(
                    ('string_encode(a%d, "utf-8") + string_encode("\\0", "utf-8")' % i)
                    if i in char_ptr else ("a%d" % i)
                    for i in range(len(fn["args"])))
                lines.append("%s %s_s(%s) {" % (rettype, name, sparams))
                lines.append("    %s%s(%s);" % (ret_kw, name, sargs))
                lines.append("}")
        lines.append("")
    return "\n".join(lines) + "\n"


def emit_structs(structs):
    lines = [
        "// GENERATED by tools/ffi/generate.py -- do not edit by hand.",
        "// Struct layouts for package_ffi: ffi_struct_layout() field-type",
        "// arrays and symbolic field offsets. See docs/driver/ffi.md.",
        "#ifndef _FFI_GENERATED_STRUCTS_H_",
        "#define _FFI_GENERATED_STRUCTS_H_",
        "#include <ffi.h>",
        "",
    ]
    for st in structs:
        up = st["name"].upper()
        codes = ", ".join(f["code"] for f in st["fields"])
        lines.append("#define STRUCT_%s_TYPES ({ %s })" % (up, codes))
        for idx, f in enumerate(st["fields"]):
            lines.append("#define STRUCT_%s_%s %d" % (up, f["name"], idx))
        lines.append("")
    lines.append("#endif")
    return "\n".join(lines) + "\n"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("header")
    ap.add_argument("--lib", default="")
    ap.add_argument("--out", required=True)
    ap.add_argument("--string-convenience", action="store_true")
    ap.add_argument("--emit-json", action="store_true")
    args = ap.parse_args()

    with open(args.header) as f:
        src = strip_comments(f.read())
    funcs, fskip = parse_functions(src)
    structs, sskip = parse_structs(src)

    with open(args.out + ".lpc", "w") as f:
        f.write(emit_lpc(funcs, args.lib, args.string_convenience))
    with open(args.out + "_structs.h", "w") as f:
        f.write(emit_structs(structs))

    if args.emit_json:
        with open(args.out + ".json", "w") as f:
            json.dump({"functions": funcs, "structs": structs}, f, indent=2)

    print("ffi-gen: %d functions, %d structs -> %s.lpc / %s_structs.h"
          % (len(funcs), len(structs), args.out, args.out))
    for name, why in fskip:
        print("  skipped function %s (%s)" % (name, why), file=sys.stderr)
    for name in sskip:
        print("  skipped struct %s (unsupported field)" % name, file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
