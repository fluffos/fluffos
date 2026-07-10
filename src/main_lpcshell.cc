// lpcshell — an interactive, Python-shell-like REPL for LPC.
//
// Boots the same driver runtime as `lpcc` (config file, master object,
// simul_efun, mudlib), then instead of loading one file and exiting, reads
// lines from stdin and evaluates them immediately.
//
// v1 design notes (the fuller
// writeup):
//
//   - Each statement is compiled as its own fresh in-memory object (via
//     simulate.cc's load_object_from_source()), wrapping the entered text
//     as the body of a synthetic function. This is the "restart pattern":
//     it reuses 100% of the existing, fully-tested compile_file() pipeline
//     unchanged, at the cost of not sharing a persistent symbol table
//     across statements (that would need real compiler surgery -- see the
//     plan's open Phase 3 item). A bare expression like `1 + 1` is tried
//     first (auto-printed, like Python); if that fails to parse, the same
//     text is retried wrapped as a statement body instead (no auto-print).
//
//   - Variable VALUES persist across statements via the existing
//     save_variable()/restore_variable() efuns, called from LPC source
//     text generated for each attempt (not by touching their C++
//     implementations directly): every REPL session tracks the set of
//     variable names it has ever seen declared, redeclares them all as
//     `mixed` globals in every subsequent statement's synthetic object, and
//     restores/re-saves their values around each call. A new variable
//     declaration (`int x = 5;`) is detected with a simple regex, added to
//     that persistent declaration set, and rewritten (type keyword
//     stripped) into a plain assignment so it targets the new global
//     rather than shadowing it with a function-local of the same name.
//     Known limitation: only single-variable declarations are recognized;
//     `int x, y;` on one line is not.
//
//   - "Is this input complete yet, or does it need a continuation line?" is
//     answered by a small hand-rolled character scanner (not the real
//     lexer): track bracket/paren/brace depth and whether we're inside a
//     string/char literal or comment, skipping over those correctly.
//     Heredocs are not tracked by this scanner (a rare thing to type
//     interactively) -- a heredoc spanning multiple lines will report a
//     compile error instead of prompting for more input. Good enough for a
//     v1 REPL; not a general incremental parser.

#include "base/std.h"

#include <cctype>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <regex>
#include <string>
#include <vector>

#include "compiler/internal/compiler.h"
#include "mainlib.h"
#include "vm/vm.h"
#include "vm/internal/apply.h"
#include "vm/internal/simulate.h"
#include "vm/internal/base/interpret.h"
#include "vm/internal/base/object.h"
#include "vm/internal/base/svalue.h"
#include "vm/internal/base/array.h"
#include "packages/core/sprintf.h"

namespace {

// ---------------------------------------------------------------------------
// Completeness check
// ---------------------------------------------------------------------------

bool LooksComplete(const std::string& s) {
  int paren = 0, brace = 0, bracket = 0;
  size_t i = 0;
  while (i < s.size()) {
    char c = s[i];
    if (c == '/' && i + 1 < s.size() && s[i + 1] == '/') {
      while (i < s.size() && s[i] != '\n') i++;
      continue;
    }
    if (c == '/' && i + 1 < s.size() && s[i + 1] == '*') {
      i += 2;
      while (i + 1 < s.size() && !(s[i] == '*' && s[i + 1] == '/')) i++;
      if (i + 1 >= s.size()) return false;  // unterminated block comment
      i += 2;
      continue;
    }
    if (c == '"' || c == '\'' || c == '`') {
      char quote = c;
      i++;
      while (i < s.size() && s[i] != quote) {
        if (s[i] == '\\' && i + 1 < s.size()) i++;
        i++;
      }
      if (i >= s.size()) return false;  // unterminated string/char/template
      i++;
      continue;
    }
    switch (c) {
      case '(':
        paren++;
        break;
      case ')':
        paren--;
        break;
      case '{':
        brace++;
        break;
      case '}':
        brace--;
        break;
      case '[':
        bracket++;
        break;
      case ']':
        bracket--;
        break;
      default:
        break;
    }
    i++;
  }
  // Negative depth (more closes than opens) means genuinely malformed
  // input, not "needs more" -- let the compiler report that error rather
  // than waiting forever for a continuation line that would never balance.
  return paren <= 0 && brace <= 0 && bracket <= 0;
}

// ---------------------------------------------------------------------------
// REPL session state: persistent variable declarations + saved values.
// ---------------------------------------------------------------------------

struct Session {
  std::vector<std::string> var_names;     // declaration order
  std::vector<std::string> saved_values;  // save_variable() strings, parallel to var_names
  int counter = 0;

  std::string Preamble() const {
    std::string out;
    for (const auto& name : var_names) {
      out += "mixed " + name + ";\n";
    }
    return out;
  }

  std::string RestoreStatements() const {
    std::string out;
    for (size_t i = 0; i < var_names.size(); i++) {
      if (saved_values[i].empty()) continue;
      out += var_names[i] + " = restore_variable(\"" + EscapeForStringLiteral(saved_values[i]) +
             "\");\n";
    }
    return out;
  }

  std::string SnapshotExpr() const {
    std::string out = "({";
    for (size_t i = 0; i < var_names.size(); i++) {
      if (i) out += ", ";
      out += "save_variable(" + var_names[i] + ")";
    }
    out += "})";
    return out;
  }

  static std::string EscapeForStringLiteral(const std::string& s) {
    std::string out;
    for (char c : s) {
      if (c == '"' || c == '\\') out += '\\';
      out += c;
    }
    return out;
  }
};

// Recognizes a single-variable declaration line: `<type> [*] name [= rest];`
// Returns true and fills `name`/`rest_stmt` (the same statement with the
// leading type keyword stripped, so it becomes a plain assignment/no-op
// targeting a global instead of redeclaring a function-local) on match.
bool MatchSingleDeclaration(const std::string& stmt, std::string* name, std::string* rest_stmt) {
  static const std::regex re(
      R"(^\s*(?:int|float|string|object|mixed|mapping|function|buffer)\s*\*?\s*([A-Za-z_]\w*)\s*(=.*)?;\s*$)");
  std::smatch m;
  if (!std::regex_match(stmt, m, re)) return false;
  *name = m[1].str();
  *rest_stmt = m[2].matched ? (*name + " " + m[2].str() + ";") : (*name + ";");
  return true;
}

// ---------------------------------------------------------------------------
// Compile-and-run one attempt. Returns the applied function's result via
// `*result` (only meaningful when `want_result` is true and this returns
// true) and updates `session`'s saved variable values on success. Throws
// nothing -- LPC-level errors are caught and reported via return false.
// ---------------------------------------------------------------------------

bool RunAttempt(Session* session, const std::string& body_src, bool want_result,
                std::string* printed_result, std::string* error_message) {
  std::string name = "/lpcshell#" + std::to_string(session->counter++);

  // __lpcshell_snapshot() is defined FIRST and __lpcshell_eval() (which
  // contains the user's just-typed, possibly-erroneous code) LAST: a
  // semantic error partway through one function's body has been observed
  // to corrupt the parser's recovery enough to garble whatever function
  // textually follows it in the same compile unit (a bogus "unexpected
  // '}'" pointing at the next function's signature). Putting the
  // user's code last means there's nothing after it left to corrupt.
  std::string src = session->Preamble();
  src += "mixed *__lpcshell_snapshot() {\n  return " + session->SnapshotExpr() + ";\n}\n";
  src += "mixed __lpcshell_eval() {\n";
  src += session->RestoreStatements();
  src += body_src;
  src += "\n}\n";

  object_t* ob = nullptr;
  error_context_t econ{};
  save_context(&econ);
  // Compile quietly: the driver would otherwise print every diagnostic to
  // the console the moment it's found -- including those of a doomed
  // trial-expression parse we're about to retry in statement form. We
  // render compiler_diags ourselves, only for the attempt that mattered.
  compiler_diags_quiet = true;
  try {
    // coverity[wrapper_escape] - the callee copies the name (shared string);
    // the c_str() pointer is not retained past this call.
    ob = load_object_from_source(src, name.c_str(), 1);
  } catch (const char* e) {
    compiler_diags_quiet = false;
    restore_context(&econ);
    pop_context(&econ);
    *error_message = e ? e : "compile error";
    return false;
  }
  compiler_diags_quiet = false;
  pop_context(&econ);

  if (!ob) {
    *error_message = "compile error";
    return false;
  }

  current_object = ob;
  save_context(&econ);
  try {
    svalue_t* ret = apply("__lpcshell_eval", ob, 0, ORIGIN_DRIVER);
    if (want_result && ret) {
      char* formatted = string_print_formatted("%O", 1, ret);
      if (formatted) {
        *printed_result = formatted;
        FREE_MSTR(formatted);
      }
    }

    if (!(ob->flags & O_DESTRUCTED) && !session->var_names.empty()) {
      svalue_t* snap = apply("__lpcshell_snapshot", ob, 0, ORIGIN_DRIVER);
      if (snap && snap->type == T_ARRAY) {
        array_t* arr = snap->u.arr;
        for (int i = 0; i < arr->size && static_cast<size_t>(i) < session->saved_values.size();
             i++) {
          if (arr->item[i].type == T_STRING) {
            session->saved_values[i] = arr->item[i].u.string;
          }
        }
      }
    }
  } catch (const char* e) {
    restore_context(&econ);
    pop_context(&econ);
    if (!(ob->flags & O_DESTRUCTED)) destruct_object(ob);
    *error_message = e ? e : "runtime error";
    return false;
  }
  pop_context(&econ);

  if (!(ob->flags & O_DESTRUCTED)) destruct_object(ob);
  return true;
}

// Strips one trailing ';' (and surrounding whitespace) if present, so a
// bare expression the user terminated with a semicolon (LPC habit) doesn't
// end up as `return (expr;);`, which would be a syntax error.
std::string StripTrailingSemicolon(std::string s) {
  size_t end = s.find_last_not_of(" \t\r\n");
  if (end != std::string::npos && s[end] == ';') {
    s.erase(end);
  }
  return s;
}

// Returns false when the statement failed to compile or errored at
// runtime, so script mode (issue #921) can report a nonzero exit status.
bool Eval(Session* session, std::string stmt) {
  // Trim trailing newline the REPL loop always appends.
  while (!stmt.empty() && (stmt.back() == '\n' || stmt.back() == '\r')) stmt.pop_back();
  if (stmt.find_first_not_of(" \t") == std::string::npos) return true;

  std::string decl_name, rewritten;
  if (MatchSingleDeclaration(stmt, &decl_name, &rewritten)) {
    bool known = false;
    for (const auto& n : session->var_names) known |= (n == decl_name);
    if (!known) {
      session->var_names.push_back(std::move(decl_name));
      session->saved_values.emplace_back();
    }
    stmt = rewritten;
  }

  std::string result, error_message;

  // A leading control-flow/statement keyword can never parse as a bare
  // expression -- skip that doomed attempt outright.
  static const std::regex statement_leader(
      R"(^\s*(if|for|while|do|switch|return|break|continue|\{)\b)");
  bool try_expression_first = !std::regex_search(stmt, statement_leader);

  if (try_expression_first) {
    std::string expr_body = "return (" + StripTrailingSemicolon(stmt) + ");";
    if (RunAttempt(session, expr_body, /*want_result=*/true, &result, &error_message)) {
      std::cout << result << "\n";
      return true;
    }
    // Trial failed silently (compiles run with compiler_diags_quiet); the
    // statement-form retry below clears compiler_diags via its own
    // compile, so the doomed trial's errors never reach the user.
  }

  // Fall back to statement form (e.g. `if (x) write(...)`, assignments,
  // loops -- anything that isn't itself an expression).
  if (RunAttempt(session, stmt, /*want_result=*/false, &result, &error_message)) {
    // Success may still have produced warnings worth showing.
    for (const auto& d : compiler_diags) {
      if (d.is_warning) std::cout << render_diagnostic(d, isatty(1)) << "\n";
    }
    return true;
  }

  // Both attempts failed. Render the FINAL attempt's structured
  // diagnostics ourselves, clang-style with the full include/macro
  // provenance chains (compiler_diags holds exactly the last compile's
  // records -- each compile clears the previous one's). error_message
  // itself stays unprinted: error()'s C++-exception unwinding path always
  // throws the generic "error handler error", not the diagnostic text.
  bool printed = false;
  for (const auto& d : compiler_diags) {
    std::cout << render_diagnostic(d, isatty(1)) << "\n";
    printed = true;
  }
  if (!printed) {
    // No compile diagnostics: the failure was at runtime (the driver's
    // own error logging already reported it to the console).
  }
  return false;
}

}  // namespace

int main(int argc, char** argv) try {
  if (argc != 2 && argc != 3) {
    std::cerr << "Usage: lpcshell config_file [script_file]" << std::endl;
    return 1;
  }

  // Script mode (issue #921): with a script file argument -- or when stdin
  // is a pipe rather than a terminal -- run without banner or prompts and
  // exit nonzero if any statement failed, so lpcshell can drive LPC as a
  // scripting-language interpreter.
  std::ifstream script;
  if (argc == 3) {
    script.open(argv[2]);
    if (!script) {
      std::cerr << "lpcshell: cannot open script file: " << argv[2] << std::endl;
      return 1;
    }
  }
  std::istream& in = (argc == 3) ? static_cast<std::istream&>(script) : std::cin;
  bool const interactive = (argc == 2) && isatty(0);

  auto config = get_argument(0, argc, argv);
  init_main(config);
  vm_start();
  current_object = master_ob;

  if (interactive) {
    std::cout << "lpcshell -- interactive LPC REPL. Ctrl-D to exit.\n";
  }

  Session session;
  std::string pending;
  bool continuation = false;
  std::string line;
  int failures = 0;
  for (;;) {
    if (interactive) {
      std::cout << (continuation ? "... " : ">>> ") << std::flush;
    }
    if (!std::getline(in, line)) {
      if (interactive) {
        std::cout << "\n";
      }
      break;
    }
    pending += line;
    pending += "\n";
    if (!LooksComplete(pending)) {
      continuation = true;
      continue;
    }
    continuation = false;
    if (!Eval(&session, pending)) {
      failures++;
    }
    pending.clear();
  }
  // Leftover unbalanced input at EOF is a failure in script mode.
  if (!pending.empty() && pending.find_first_not_of(" \t\r\n") != std::string::npos) {
    if (!Eval(&session, pending)) {
      failures++;
    }
  }

  clear_state();
  return failures ? 1 : 0;
} catch (const std::exception& e) {
  std::cerr << "lpcshell: fatal: " << e.what() << std::endl;
  return 1;
}
