---
title: pcre / pcre_config
---
# pcre_config

### NAME

    pcre_config() - PCRE2 library capabilities

### SYNOPSIS

    mapping pcre_config(void);

### DESCRIPTION

    Returns what this driver build's PCRE2 library actually provides:

      version            string   e.g. `10.42 2022-12-11`
      jit                int      1 if JIT is available
      jit_target         string   JIT architecture (only when `jit` is 1)
      unicode            int      1 if Unicode / UCP support is compiled in
      unicode_version    string
      newline            int      default newline convention
      bsr                int      default `\R` convention
      link_size          int
      match_limit        int      library default
      depth_limit        int      library default
      heap_limit         int      library default (when the library reports it)
      never_backslash_c  int

    Use this to decide at runtime whether JIT or Unicode properties are
    present. The same information is what `pcre_version()` compresses into a
    single string (`"10.42 2022-12-11 JIT"`).

    **Options on the other `pcre_*` efuns.** The last argument of
    `pcre_match`, `pcre_match_all`, `pcre_extract`, `pcre_assoc`,
    `pcre_replace`, and `pcre_replace_callback` is either the integer bits
    from `src/include/pcre_flags.h` or a mapping of named PCRE2 options.
    UTF-8, Unicode properties, and JIT are on by default. Mapping keys:

    Compile: `caseless`/`i`, `multiline`/`m`, `dotall`/`s`, `ungreedy`/`u`,
    `extended`/`x`, `extended_more`, `endanchored`, `no_auto_capture`,
    `firstline`, `dupnames`, `literal`, `dollar_endonly`, `alt_bsux`,
    `alt_circumflex`, `alt_verbnames`, `allow_empty_class`,
    `match_unset_backref`, `no_auto_possess`, `no_dotstar_anchor`,
    `no_start_optimize`, `no_utf_check`, `use_offset_limit`, `ucp`,
    `no_ucp`.

    Match: `anchored`/`a`, `notempty`, `notbol`, `noteol`, `partial` /
    `partial_soft`, `partial_hard`, `notempty_atstart`.

    Substitute (`pcre_replace` string form): `replace_global` (default on),
    `replace_extended` (default on), `replace_literal`,
    `replace_replacement_only`, `replace_matched`, `replace_unknown_unset`,
    `replace_unset_empty`.

    Limits (ReDoS guards): `match_limit`, `depth_limit`, `heap_limit`,
    `offset_limit` — integers. Exceeding a limit is a catchable error.

    Start position: `offset` (byte offset into the subject).

    Compile context: `newline` (`cr`/`lf`/`crlf`/`any`/`anycrlf`/`nul`),
    `bsr` (`unicode`/`anycrlf`), `no_jit`, and `extra_*` (`extra_match_word`,
    `extra_match_line`, `extra_alt_bsux`, `extra_bad_escape_is_literal`,
    `extra_escaped_cr_is_lf`, `extra_allow_surrogate_escapes`,
    `extra_allow_lookaround_bsk`).

    Related efuns: `pcre_info(pattern)` (compiled metadata),
    `pcre_convert(pattern, (["glob": 1]))` (glob / POSIX → PCRE2).

    Not exposed, on purpose: callouts (LPC callbacks during match), DFA
    matching, and serialized bytecode. Those need C function pointers or
    raw `pcre2_code*` lifetimes.

    Combine leftover int bits with names via `flags`:

        pcre_match(s, pat, (["caseless": 1, "match_limit": 100000]))
        pcre_match(s, pat, (["flags": PCRE_I | PCRE_M]))

    Unknown keys error. Integer bits stay valid so existing mudlibs do not
    have to change.

    **Why this is an efun, not FFI or a stdlib binding.** FluffOS already
    links `libpcre2-8` into the driver and owns the compile cache, JIT,
    UTF-8 subject length, and `error()`. FFI would require `PACKAGE_FFI`,
    a `valid_ffi` allow-list, a second load of the shared library, and LPC
    to manage `pcre2_code*` / `pcre2_match_data*` lifetimes — a crash and
    leak surface. The documented stdlib helpers (`/std/ffi_util` and
    friends) are mudlib files for wrapping *other* C libraries, not a
    second regex API. Copy `src/include/pcre_flags.h` into the mudlib
    include dir and call these efuns.

### SEE ALSO

    pcre_version(3), pcre_match(3), pcre_replace(3)
