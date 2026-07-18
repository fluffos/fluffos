#ifndef STAGE_OUTPUT_H_
#define STAGE_OUTPUT_H_

#include <cstdio>

// Staged compiler outputs (lpcc -E / --tokens): drive the real lexer +
// preprocessor over `fd`'s content and print, WITHOUT parsing.
//
//   pp_form == true   preprocessed source, gcc -E shaped: macros expanded,
//                     directives applied, tokens re-rendered with source
//                     line structure preserved.
//   pp_form == false  one token per line: line:col kind spelling.
//
// The later stages are CompileState knobs consumed by compile_file():
//   g_compile.opt_dump_ast     print the parse trees before codegen
//   g_compile.opt_no_optimize  clear PRAGMA_OPTIMIZE (pre-optimization
//                              bytecode when combined with dump_prog)
//
// Returns false if the stream failed to load.
bool lpc_dump_stage_tokens(int fd, const char* name, bool pp_form, FILE* out);

// lpcc --json --tokens: same scan, machine-readable. Emits ONE line:
//   {"fluffos_lpcc":1,"stage":"tokens","file":...,
//    "tokens":[{"l":line,"c":col,"k":kind,"n":"L_IDENTIFIER","t":"spelling"},...]}
// so consumers can find the payload in mixed stdout by the "fluffos_lpcc"
// envelope marker instead of scraping the human format.
bool lpc_dump_stage_tokens_json(int fd, const char* name, FILE* out);

#endif
