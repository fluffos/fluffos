#!/usr/bin/env python3
"""Generate grammar.ebnf from a Bison-generated grammar.xml.

Usage:
    generate_ebnf.py <grammar.xml> <grammar.ebnf>

The grammar.xml is produced by CMake via:
    bison --xml=grammar.xml -o grammar_xml_dummy.cc grammar.y

This script uses xml.etree.ElementTree.iterparse() to stream the XML
without loading the entire document into memory.
"""
import sys
import os
import xml.etree.ElementTree as ET


def to_camel_case(name: str) -> str:
    if name.startswith("L_"):
        parts = name[2:].split("_")
        return "L" + "".join(p.capitalize() for p in parts)
    if "_" in name:
        parts = name.split("_")
        return parts[0] + "".join(p.capitalize() for p in parts[1:])
    return name


def is_internal(name: str) -> bool:
    """Return True for Bison-internal symbols that should not appear in EBNF."""
    return name.startswith("$") or "@" in name


def fmt_sym(sym: str) -> str:
    """Format a single grammar symbol for EBNF output."""
    if sym.startswith("'") and sym.endswith("'"):
        return f'"{sym[1:-1]}"'
    return to_camel_case(sym)


def fmt_syms(syms: list) -> str:
    """Format a sequence of grammar symbols as a comma-separated EBNF term."""
    return ", ".join(fmt_sym(s) for s in syms)


def try_fold_recursive(lhs: str, alternatives: list) -> "str | None":
    """Detect list/repetition patterns and return a folded EBNF body, or None.

    Patterns handled:
      Left-rec + empty:  A: A alpha | epsilon         ->  { alpha }
      Right-rec + empty: A: alpha A | epsilon         ->  { alpha }
      Multi-alt + empty: A: a1 A | a2 A | epsilon     ->  { a1 | a2 }
      Left-rec list:     A: A tail | base             ->  base, { tail }
      Right-rec list:    A: base sep A | base         ->  base, { sep, base }
    """
    empty_alts = [a for a in alternatives if not a]
    nonempty_alts = [a for a in alternatives if a]

    if not nonempty_alts:
        return None

    left_rec = [a for a in nonempty_alts if a[0] == lhs]
    right_rec = [a for a in nonempty_alts if a[-1] == lhs]
    non_rec = [a for a in nonempty_alts if a[0] != lhs and a[-1] != lhs]

    # All non-empty alts left-recursive + empty base  ->  { body }
    if left_rec and len(left_rec) == len(nonempty_alts) and empty_alts:
        tails = [a[1:] for a in left_rec]
        inner = " | ".join(fmt_syms(t) for t in tails)
        return "{ " + inner + " }"

    # All non-empty alts right-recursive + empty base  ->  { body }
    if right_rec and len(right_rec) == len(nonempty_alts) and empty_alts:
        heads = [a[:-1] for a in right_rec]
        inner = " | ".join(fmt_syms(h) for h in heads)
        return "{ " + inner + " }"

    # Exactly two alts, no empty: one recursive, one base  ->  base, { body }
    if len(nonempty_alts) == 2 and not empty_alts and len(non_rec) == 1:
        base = non_rec[0]
        base_s = fmt_syms(base)

        if len(left_rec) == 1:
            # A: A tail | base  ->  base, { tail }
            tail = left_rec[0][1:]
            return base_s + ", { " + fmt_syms(tail) + " }"

        if len(right_rec) == 1:
            # A: base sep A | base  ->  base, { sep, base }
            head = right_rec[0][:-1]
            if head[:len(base)] == base:
                sep = head[len(base):]
                return base_s + ", { " + fmt_syms(sep) + ", " + base_s + " }"
            # Fallback: base, { head }
            return base_s + ", { " + fmt_syms(head) + " }"

    return None


def stream_rules(xml_path: str):
    """Yield (lhs, [rhs_symbols]) from grammar.xml using iterparse (streaming)."""
    lhs = None
    rhs = []
    inside_rule = False
    inside_rhs = False

    for event, elem in ET.iterparse(xml_path, events=("start", "end")):
        tag = elem.tag

        if event == "start":
            if tag == "rule":
                lhs = None
                rhs = []
                inside_rule = True
                inside_rhs = False
            elif tag == "rhs" and inside_rule:
                inside_rhs = True

        elif event == "end":
            if tag == "lhs" and inside_rule:
                lhs = elem.text or ""
            elif tag == "symbol" and inside_rhs:
                sym = (elem.text or "").strip()
                if sym and not is_internal(sym):
                    rhs.append(sym)
            elif tag == "empty" and inside_rhs:
                pass  # empty alternative — rhs stays []
            elif tag == "rhs":
                inside_rhs = False
            elif tag == "rule":
                if lhs and not is_internal(lhs):
                    yield lhs, rhs
                inside_rule = False
                # Free the element from memory immediately
                elem.clear()


def build_ebnf(xml_path: str) -> str:
    """Stream grammar.xml and produce an ISO/IEC 14977 EBNF string."""
    # Collect: {lhs: [alternative, ...]} preserving insertion order
    rules: dict[str, list[list[str]]] = {}

    for lhs, rhs in stream_rules(xml_path):
        if lhs not in rules:
            rules[lhs] = []
        rules[lhs].append(rhs)

    lines = [
        "(* ============================================================================",
        "   LPC Grammar Specification (ISO/IEC 14977 EBNF)",
        "   Automatically generated from grammar.y. Do not edit directly.",
        "   ============================================================================ *)",
        "",
    ]

    for lhs, alternatives in rules.items():
        ebnf_lhs = to_camel_case(lhs)

        # Try to fold recursive list patterns into { ... } repetition notation
        folded = try_fold_recursive(lhs, alternatives)
        if folded is not None:
            lines.append(f"{ebnf_lhs} = {folded} ;")
            lines.append("")
            continue

        # Standard multi-alternative formatting
        formatted_alts = []
        for alt in alternatives:
            if not alt:
                formatted_alts.append('""')
            else:
                formatted_alts.append(fmt_syms(alt))

        if len(formatted_alts) == 1:
            lines.append(f"{ebnf_lhs} = {formatted_alts[0]} ;")
        else:
            prefix = f"{ebnf_lhs} = "
            indent = " " * len(prefix)
            for i, alt in enumerate(formatted_alts):
                if i == 0:
                    lines.append(f"{prefix}{alt}")
                else:
                    lines.append(f"{indent}| {alt}")
            lines[-1] += " ;"

        lines.append("")

    return "\n".join(lines)


# ---------------------------------------------------------------------------
# Machine-readable grammar contract (lpc-grammar.json).
#
# TOKEN_SPEC categorizes every terminal grammar.y uses and records its
# concrete source spellings (source of truth: lexer.l's operator rules and
# lexer_utils.cc's reserved-word table). emit_grammar_json() asserts full
# coverage of the XML's terminal list, so adding a token to grammar.y
# without updating this spec is a regeneration ERROR -- the freshness
# guarantee for the JS tooling built on the JSON.
# ---------------------------------------------------------------------------
TOKEN_SPEC = {
    # literals / names
    "L_STRING": ("string", None), "L_NUMBER": ("number", None),
    "L_REAL": ("number", None),
    "L_TEMPLATE_HEAD": ("string", None), "L_TEMPLATE_MIDDLE": ("string", None),
    "L_TEMPLATE_TAIL": ("string", None),
    "L_IDENTIFIER": ("identifier", None), "L_DEFINED_NAME": ("identifier", None),
    "L_PARAMETER": ("identifier", None),
    # keyword families (spellings from lexer_utils.cc reswords[])
    "L_BASIC_TYPE": ("type", ["buffer", "float", "function", "int", "mapping",
                              "mixed", "object", "string", "void"]),
    "L_TYPE_MODIFIER": ("modifier", ["nomask", "nosave", "private", "protected",
                                     "public", "static", "varargs"]),
    "L_IF": ("keyword", ["if"]), "L_ELSE": ("keyword", ["else"]),
    "L_SWITCH": ("keyword", ["switch"]), "L_CASE": ("keyword", ["case"]),
    "L_DEFAULT": ("keyword", ["default"]), "L_WHILE": ("keyword", ["while"]),
    "L_DO": ("keyword", ["do"]), "L_FOR": ("keyword", ["for"]),
    "L_FOREACH": ("keyword", ["foreach"]), "L_IN": ("keyword", ["in"]),
    "L_BREAK": ("keyword", ["break"]), "L_CONTINUE": ("keyword", ["continue"]),
    "L_RETURN": ("keyword", ["return"]), "L_INHERIT": ("keyword", ["inherit"]),
    "L_CATCH": ("keyword", ["catch"]), "L_NEW": ("keyword", ["new"]),
    "L_CLASS": ("keyword", ["class"]), "L_EFUN": ("keyword", ["efun"]),
    "L_SSCANF": ("keyword", ["sscanf"]),
    "L_PARSE_COMMAND": ("keyword", ["parse_command"]),
    "L_TIME_EXPRESSION": ("keyword", ["time_expression"]),
    "L_ARRAY": ("keyword", ["array"]), "L_REF": ("keyword", ["ref"]),
    "L_TREE": ("keyword", ["__TREE__"]),
    # multi-character operators (spellings from lexer.l)
    "L_ASSIGN": ("operator", ["=", "+=", "-=", "*=", "/=", "%=", "&=", "|=",
                              "^=", "<<=", ">>=", "&&=", "||=", "??="]),
    "L_INC_DEC": ("operator", ["++", "--"]),
    "L_LAND": ("operator", ["&&"]), "L_LOR": ("operator", ["||"]),
    "L_QUESTION_QUESTION": ("operator", ["??"]),
    "L_SHIFT": ("operator", ["<<", ">>"]),
    "L_EQ_NE": ("operator", ["==", "!="]),
    "L_ORDER": ("operator", [">", ">=", "<="]),
    "L_ARROW": ("operator", ["->"]), "L_COLON_COLON": ("operator", ["::"]),
    "L_RANGE": ("operator", [".."]), "L_DOT_DOT_DOT": ("operator", ["..."]),
    "L_OPTIONAL_DOT": ("operator", ["?."]), "L_DOT_OPTIONAL": ("operator", [".?"]),
    "L_FUNCTION_OPEN": ("operator", ["(:"]),
    # bison-internal
    "$end": ("internal", None), "error": ("internal", None),
    "LOWER_THAN_ELSE": ("internal", None),
}

PREPROCESSOR_DIRECTIVES = ["define", "undef", "include", "if", "ifdef",
                           "ifndef", "elif", "else", "endif", "pragma",
                           "line", "echo", "error", "warn"]
BUILTIN_MACROS = ["__LINE__", "__FILE__", "__DIR__"]


def emit_grammar_json(xml_path, json_path):
    import json
    tree = ET.parse(xml_path)
    terminals = []
    for t in tree.iter("terminal"):
        name = t.get("name")
        if name:
            terminals.append(name)

    keywords, types, modifiers, operators, punctuation = [], [], [], [], []
    tokens = {}
    unknown = []
    for name in terminals:
        if name.startswith("'") and name.endswith("'"):
            punctuation.append(name[1:-1])
            continue
        spec = TOKEN_SPEC.get(name)
        if spec is None:
            unknown.append(name)
            continue
        category, spellings = spec
        tokens[name] = {"category": category}
        if spellings:
            tokens[name]["spellings"] = spellings
            if category == "keyword":
                keywords.extend(spellings)
            elif category == "type":
                types.extend(spellings)
            elif category == "modifier":
                modifiers.extend(spellings)
            elif category == "operator":
                operators.extend(spellings)
    if unknown:
        print("ERROR: grammar.y grew tokens the grammar spec does not know: "
              + ", ".join(sorted(unknown))
              + "\n       Update TOKEN_SPEC in generate_ebnf.py (and the "
              + "lexical layer / JS tooling if user-visible).", file=sys.stderr)
        sys.exit(1)

    # Longest-match order for tokenizers; ":)" closes "(:" functionals.
    operators = sorted(set(operators + [":)"]), key=len, reverse=True)

    productions = []
    for rule in tree.iter("rule"):
        lhs = rule.findtext("lhs")
        if lhs is None or is_internal(lhs):
            continue
        rhs = [sym.text for sym in rule.find("rhs")
               if sym.tag == "symbol" and sym.text and not is_internal(sym.text)]
        productions.append({"lhs": lhs, "rhs": rhs})

    data = {
        "version": 1,
        "generator": "src/compiler/internal/generate_ebnf.py",
        "keywords": sorted(set(keywords)),
        "typeKeywords": sorted(set(types)),
        "modifierKeywords": sorted(set(modifiers)),
        "operators": operators,
        "punctuation": sorted(set(punctuation)),
        "directives": PREPROCESSOR_DIRECTIVES,
        "builtinMacros": BUILTIN_MACROS,
        "tokens": tokens,
        "productions": productions,
    }
    print(f"Writing {json_path} ...")
    with open(json_path, "w") as f:
        json.dump(data, f, indent=2, sort_keys=False)
        f.write("\n")


def main():
    if len(sys.argv) not in (3, 4):
        print(f"Usage: {sys.argv[0]} <grammar.xml> <grammar.ebnf> [lpc-grammar.json]", file=sys.stderr)
        sys.exit(1)

    xml_path  = sys.argv[1]
    ebnf_path = sys.argv[2]
    json_path = sys.argv[3] if len(sys.argv) > 3 else None
    lexical_path = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                "grammar_lexical.ebnf.in")

    if not os.path.isfile(xml_path):
        print(f"ERROR: XML file not found: {xml_path}", file=sys.stderr)
        sys.exit(1)

    print(f"Streaming {xml_path} ...")
    syntax_layer = build_ebnf(xml_path)

    # Compose the three layers: hand-authored lexical + preprocessor
    # (grammar_lexical.ebnf.in -- regeneration can never lose them), then
    # the bison-derived syntax layer.
    with open(lexical_path) as f:
        lexical_layer = f.read()
    ebnf_content = (lexical_layer.rstrip() + "\n\n"
                    + "(* " + "=" * 76 + "\n"
                    + "   Layer 3: SYNTAX GRAMMAR (generated from grammar.y via bison --xml).\n"
                    + "   Do not edit -- regenerate with the generate_ebnf CMake target.\n"
                    + "   " + "=" * 76 + " *)\n\n"
                    + syntax_layer.split("*)", 1)[1].lstrip("\n"))

    # Machine-readable contract for JS tooling (lpc-grammar.json):
    # verifies every terminal in grammar.y is categorized -- a NEW token
    # without a spec entry fails regeneration loudly, so the artifact can
    # never silently go stale again.
    if json_path is not None:
        emit_grammar_json(xml_path, json_path)

    # Optional validation guard
    try:
        import parse_ebnf.parsing
        parse_ebnf.parsing.parse_string(ebnf_content)
        print("SUCCESS: Generated EBNF is 100% valid (ISO/IEC 14977).")
    except ImportError:
        print("WARNING: 'parse-ebnf' not installed — skipping validation.")
    except Exception as exc:
        print(f"EBNF SYNTAX ERROR: {exc}", file=sys.stderr)
        sys.exit(1)

    print(f"Writing {ebnf_path} ...")
    with open(ebnf_path, "w") as f:
        f.write(ebnf_content)

    print("Done.")


if __name__ == "__main__":
    main()
