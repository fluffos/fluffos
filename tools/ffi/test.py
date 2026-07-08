#!/usr/bin/env python3
"""Dependency-free tests for the FFI bindings generator.

    python3 tools/ffi/test.py

Generates bindings from test_sample.h and asserts the emitted LPC/struct
output and the parsed JSON contract.
"""
import json
import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
GEN = os.path.join(HERE, "generate.py")
SAMPLE = os.path.join(HERE, "test_sample.h")

failures = 0


def check(name, cond, detail=""):
    global failures
    if cond:
        print("  OK  " + name)
    else:
        print("FAIL  " + name + ((" -- " + detail) if detail else ""))
        failures += 1


def main():
    with tempfile.TemporaryDirectory() as d:
        out = os.path.join(d, "sample")
        res = subprocess.run(
            [sys.executable, GEN, SAMPLE, "--lib", "libm.so.6", "--out", out,
             "--string-convenience", "--emit-json"],
            capture_output=True, text=True)
        check("generator exits 0", res.returncode == 0, res.stderr)

        lpc = open(out + ".lpc").read()
        structs = open(out + "_structs.h").read()
        data = json.load(open(out + ".json"))

        names = [f["name"] for f in data["functions"]]
        check("supported functions parsed",
              set(["sqrt", "pow", "abs", "strlen", "qsort"]).issubset(set(names)), str(names))
        check("varargs skipped", "printf" not in names)
        check("function-pointer param skipped", "set_handler" not in names)

        # sqrt(double)->double becomes float sqrt(float ...).
        check("scalar wrapper shape", "float sqrt(float a0)" in lpc, lpc)
        check("double return code", '"sqrt", FFI_DOUBLE, ({ FFI_DOUBLE })' in lpc)
        check("pow two-arg codes", "FFI_DOUBLE, FFI_DOUBLE" in lpc)
        check("int abs mapped", "int abs(int a0)" in lpc)

        # char* becomes a buffer parameter (never string).
        check("char* param is buffer", "int strlen(buffer a0)" in lpc, lpc)
        check("strlen return code UINT64", '"strlen", FFI_UINT64' in lpc)
        # ...with a string-convenience overload that encodes UTF-8.
        check("string-convenience overload emitted", "strlen_s(string a0)" in lpc)
        check("convenience encodes utf-8", 'string_encode(a0, "utf-8")' in lpc)

        # void return: no `return`.
        check("void wrapper has no return", "void qsort(" in lpc and "return ffi_call(_h_qsort" not in lpc)

        # Struct layout: field-type array + symbolic offsets.
        check("Point struct types", "#define STRUCT_POINT_TYPES ({ FFI_INT32, FFI_INT32 })" in structs)
        check("Point field indices", "#define STRUCT_POINT_x 0" in structs and
              "#define STRUCT_POINT_y 1" in structs)
        check("Mixed struct types",
              "#define STRUCT_MIXED_TYPES ({ FFI_INT8, FFI_DOUBLE, FFI_INT16 })" in structs)

        # JSON contract mirrors the emitted bindings.
        sqrt_fn = next(f for f in data["functions"] if f["name"] == "sqrt")
        check("json ret code", sqrt_fn["ret"][0] == "FFI_DOUBLE")
        check("json struct fields",
              any(s["name"] == "Point" and [x["name"] for x in s["fields"]] == ["x", "y"]
                  for s in data["structs"]))

    print("\nAll ffi-gen tests passed." if failures == 0 else "\n%d FAILURES" % failures)
    return 0 if failures == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
