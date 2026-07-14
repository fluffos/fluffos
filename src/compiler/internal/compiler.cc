#include "base/std.h"

#include "compiler.h"
#include "compiler/internal/compiler_utils.h"

#include <cstdlib>  // for qsort
#include <cstdio>   // for sprintf

#include <fmt/format.h>

#include "efuns.autogen.h"          // FIXME
#include "applies_table.autogen.h"  // FIXME:

#include "vm/internal/base/program.h"
#include "vm/internal/base/svalue.h"
#include "generate.h"
#include "icode.h"
#include "lexer.h"
#include "compiler/internal/lexer_utils.h"
#include "compiler/internal/grammar_rules.h"
#include "grammar.autogen.h"
#include "scratchpad.h"
#include "symbol.h"
#include <string>
#include <utility>
#include <vector>

#include "vm/internal/base/machine.h"  // for error(), FIXME

#ifdef PACKAGE_MUDLIB_STATS
#include "packages/mudlib_stats/mudlib_stats.h"
#endif

// Align to pointer-size boundary, this will work fine for both x86 & x86_64, because we don't use
// long double (16-bytes).
#define align(x) (((x) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1))

/*
 * 'inherit_file' is used as a flag. If it is set to a string
 * after yyparse(), this string should be loaded as an object,
 * and the original object must be loaded again.
 */
char* inherit_file;

/*
 * When master::inherit_program returns the inherited program's source
 * inline (an array of strings), rule_inheritence() joins it here,
 * alongside setting 'inherit_file'. load_object() consumes both
 * together: a non-empty source compiles via load_object_from_source()
 * under the 'inherit_file' name instead of reading the file from disk.
 */
std::string inherit_file_source;

// FIXME: this is defined in vm/internal/simul_efun.cc
extern object_t* simul_efun_ob;
// FIXME: This is used by smart_log().cc
extern svalue_t* safe_apply_master_ob(int, int);

static void clean_parser(void);
static bool prolog(std::string_view source, const char* /*name*/, void* scanner);
// When armed, prolog loads the token stream from this fd (zero copy)
// instead of the source view -- an explicit flag, NOT an fd sentinel, so
// a caller's invalid fd (e.g. -1) still flows through the fd path and
// fails properly instead of silently compiling the empty view. Set by
// compile_file_fd for one compile; consumed-and-cleared by prolog
// (unwind-safe). The compiler is single-threaded and non-reentrant.
static int prolog_source_fd = -1;
static bool prolog_source_is_fd = false;
static program_t* epilog(void);
static void show_overload_warnings(void);

#define CT(x) (1 << (x))
#define CT_SIMPLE(x) (CT(TYPE_ANY) | CT(x))

short compatible[11] = {
    /* UNKNOWN */ 0,
    /* ANY */ 0xfff,
    /* NOVALUE to*/ CT_SIMPLE(TYPE_NOVALUE) | CT(TYPE_VOID) | CT(TYPE_NUMBER),
    /* VOID to*/ CT_SIMPLE(TYPE_VOID) | CT(TYPE_NUMBER),
    /* NUMBER to*/ CT_SIMPLE(TYPE_NUMBER) | CT(TYPE_REAL),
    /* STRING */ CT_SIMPLE(TYPE_STRING),
    /* OBJECT */ CT_SIMPLE(TYPE_OBJECT),
    /* MAPPING */ CT_SIMPLE(TYPE_MAPPING),
    /* FUNCTION */ CT_SIMPLE(TYPE_FUNCTION),
    /* REAL */ CT_SIMPLE(TYPE_REAL) | CT(TYPE_NUMBER),
    /* BUFFER */ CT_SIMPLE(TYPE_BUFFER),
};

short is_type[11] = {
    /* UNKNOWN */ 0,
    /* ANY */ 0xfff,
    /* NOVALUE */ CT_SIMPLE(TYPE_NOVALUE) | CT(TYPE_VOID),
    /* VOID */ CT_SIMPLE(TYPE_VOID) | CT(TYPE_NOVALUE),
    /* NUMBER */ CT_SIMPLE(TYPE_NUMBER),
    /* STRING */ CT_SIMPLE(TYPE_STRING),
    /* OBJECT */ CT_SIMPLE(TYPE_OBJECT),
    /* MAPPING */ CT_SIMPLE(TYPE_MAPPING),
    /* FUNCTION */ CT_SIMPLE(TYPE_FUNCTION),
    /* REAL */ CT_SIMPLE(TYPE_REAL),
    /* BUFFER */ CT_SIMPLE(TYPE_BUFFER),
};

mem_block_t mem_block[NUMAREAS];

parse_node_t* comp_trees[NUMTREES];
int comp_last_inherited;
int current_tree;

function_context_t function_context;

/* only lower 16bits are used */
int exact_types, global_modifiers;

/* 16bits modifier + 16bits type */
int current_type;

int var_defined;

unsigned short *comp_def_index_map, *func_index_map;
unsigned short *prog_flags, *comp_sorted_funcs;

char* prog_code;
char* prog_code_max;

// See compiler.h: null (the boot-time/unit-test default) means the compiler
// must never touch the VM (eval-stack pushes, master applies, mudlib stats);
// compile_file() sets it from its vm_context parameter for the compile's
// duration and restores it on the way out.
vm_context_t g_driver_vm_context;

// THE compiler state object (see compiler.h). All the legacy spellings
// (compiler_diags, compiler_vm_context, current_session, ...) are inline
// references into this.
CompileState g_compile;

void rule_set_operand_ranges(int l1, int c1, int e1, int lop, int cop, int l2, int c2, int e2) {
  compiler_pending_ranges.clear();
  compiler_pending_ranges.push_back(Diagnostic::Range{l1, c1, e1});
  compiler_pending_ranges.push_back(Diagnostic::Range{l2, c2, e2});
  compiler_pending_caret_line = lop;
  compiler_pending_caret_col = cop;
}
void rule_clear_operand_ranges(void) {
  compiler_pending_ranges.clear();
  compiler_pending_caret_line = 0;
  compiler_pending_caret_col = 0;
}

// Displays a compiler-internal filename (stored without a leading slash)
// the way the mudlib names it.
static std::string display_path(const std::string& file) {
  if (!file.empty() && file[0] == '/') return file;
  return "/" + file;
}

// Fetch one line of a mudlib-relative source file for a diagnostic
// snippet (macro-definition notes). The driver runs chdir()ed into the
// mudlib root, so the real path is the mud path without its leading '/'.
// Best-effort: returns "" when unreadable (in-memory sources, gone files).
static std::string read_source_line(const char* mud_path, int line_no) {
  if (mud_path == nullptr || line_no <= 0) return "";
  const char* rel = mud_path[0] == '/' ? mud_path + 1 : mud_path;
  FILE* f = fopen(rel, "rb");
  if (f == nullptr) return "";
  std::string line;
  int cur = 1;
  int ch;
  while ((ch = fgetc(f)) != EOF) {
    if (cur == line_no) {
      if (ch == '\n') break;
      if (line.size() < 512) line += static_cast<char>(ch);
    } else if (ch == '\n') {
      cur++;
      if (cur > line_no) break;
    }
  }
  fclose(f);
  while (!line.empty() && (line.back() == '\r')) line.pop_back();
  return line;
}

std::string render_diagnostic(const Diagnostic& d, bool color) {
  const char* c_err = color ? "\033[1;31m" : "";
  const char* c_warn = color ? "\033[1;35m" : "";
  const char* c_note = color ? "\033[1;30m" : "";
  const char* c_bold = color ? "\033[1m" : "";
  const char* c_caret = color ? "\033[1;32m" : "";
  const char* c_off = color ? "\033[0m" : "";
  std::string out;

  // Gutter snippet, clang-shaped:
  //   %5d | <source line>
  //         | <marks>
  auto emit_snippet = [&](int line_no, std::string shown, int caret_col,
                          const std::vector<Diagnostic::Range>* ranges,
                          const std::vector<Diagnostic::FixIt>* fixits) {
    for (auto& ch : shown) {
      if (ch == '\t') ch = ' ';  // tab as one column, matching capture
    }
    char num[16];
    snprintf(num, sizeof(num), "%5d", line_no);
    out += "\n";
    out += num;
    out += " | " + shown;
    std::string marks(shown.size() + 1, ' ');
    bool any = false;
    if (ranges != nullptr) {
      for (const auto& r : *ranges) {
        if (r.line != line_no || r.col_start <= 0 || r.col_start > r.col_end) continue;
        for (int c = r.col_start; c <= r.col_end && static_cast<size_t>(c) <= marks.size(); c++) {
          marks[static_cast<size_t>(c - 1)] = '~';
          any = true;
        }
      }
    }
    if (caret_col > 0 && static_cast<size_t>(caret_col) <= shown.size() + 1) {
      marks[static_cast<size_t>(caret_col - 1)] = '^';
      any = true;
    }
    if (any) {
      while (!marks.empty() && marks.back() == ' ') marks.pop_back();
      out += "\n      | ";
      out += c_caret;
      out += marks;
      out += c_off;
    }
    if (fixits != nullptr) {
      for (const auto& f : *fixits) {
        if (f.col_start > 0 && static_cast<size_t>(f.col_start) <= shown.size() + 1) {
          out += "\n      | " + std::string(static_cast<size_t>(f.col_start - 1), ' ');
          out += c_caret;
          out += f.replacement;
          out += c_off;
        }
      }
    }
  };

  // clang's convention: the include chain prints BEFORE the main line,
  // outermost includer first (capture stores innermost first).
  for (auto it = d.included_from.rbegin(); it != d.included_from.rend(); ++it) {
    out += "In file included from " + display_path(it->first) + ":" + std::to_string(it->second) +
           ":\n";
  }
  out += display_path(d.file);
  out += ":" + std::to_string(d.line);
  if (d.column > 0) {
    out += ":" + std::to_string(d.column);
  }
  out += ": ";
  out += d.is_warning ? c_warn : c_err;
  out += d.is_warning ? "warning: " : "error: ";
  out += c_off;
  out += c_bold;
  out += d.message;
  out += c_off;
  if (!d.snippet.empty()) {
    emit_snippet(d.line, d.snippet, d.column, &d.ranges, &d.fixits);
  }

  // Macro-expansion cascade, clang-shaped: one located note per level,
  // carrying the macro DEFINITION line with a caret at the name.
  for (const auto& exp : d.expansions) {
    out += "\n";
    if (!exp.def_file.empty()) {
      std::string def_line_text = read_source_line(exp.def_file.c_str(), exp.def_line);
      size_t name_pos = def_line_text.find(exp.macro_name);
      int def_col = name_pos == std::string::npos ? 0 : static_cast<int>(name_pos) + 1;
      out += display_path(exp.def_file) + ":" + std::to_string(exp.def_line);
      if (def_col > 0) out += ":" + std::to_string(def_col);
      out += ": ";
      out += c_note;
      out += "note: ";
      out += c_off;
      out += "expanded from macro '" + exp.macro_name + "'";
      if (!def_line_text.empty()) {
        emit_snippet(exp.def_line, def_line_text, def_col, nullptr, nullptr);
      }
    } else {
      out += c_note;
      out += "note: ";
      out += c_off;
      out += "expanded from builtin macro '" + exp.macro_name + "'";
    }
  }
  for (const auto& note : d.notes) {
    out += "\n  note: ";
    out += note;
  }
  return out;
}

// Builds and stores the structured record for one reported diagnostic --
// called by yyerror()/yywarn() at the exact moment of the report, because
// the provenance is LIVE state: the #include stack is popped as includes
// finish, the macro-expansion frames are popped as splices are consumed,
// and the session chain is popped as compiles finish, so capturing later
// would see less. Returns the stored record for the caller to print.
static const Diagnostic& capture_diagnostic(bool is_warning, const char* message) {
  Diagnostic d;
  d.is_warning = is_warning;
  d.file = current_file != nullptr ? current_file : "";
  d.line = compiler_directive_start_line != 0 ? compiler_directive_start_line : current_line;
  d.message = message;
  d.included_from = lpc_lex_include_stack();
  auto chain = lpc_lex_expansion_chain();
  for (const auto& site : chain) {
    d.expansions.push_back(Diagnostic::Expansion{site.name, site.def_file, site.def_line,
                                                 site.invocation_line, site.invocation_column});
  }
  // Column + snippet (8.1/8.2). Inside an expansion, attribute to the
  // OUTERMOST invocation (chain is innermost-first, so .back()) -- same
  // frame `line` already reads. A directive-attributed report (whole-line
  // constructs) carries no useful token column.
  if (compiler_directive_start_line == 0) {
    if (!chain.empty()) {
      d.column = chain.back().invocation_column;
    } else if (void* scanner = lpc_lex_active_scanner()) {
      d.column = yyget_extra(scanner)->token_start_column + 1;
    }
    d.snippet = lpc_lex_current_source_line();
  }
  d.notes = std::move(compiler_pending_notes);
  compiler_pending_notes.clear();
  d.fixits = std::move(compiler_pending_fixits);
  compiler_pending_fixits.clear();
  if (!compiler_current_load_reason.empty()) {
    d.notes.push_back(compiler_current_load_reason);
  }
  d.ranges = compiler_pending_ranges;  // copied, not moved: the owning
                                       // grammar action clears them
  if (compiler_pending_caret_line != 0 && compiler_pending_caret_line == d.line) {
    d.column = compiler_pending_caret_col;  // caret on the operator itself
  }
  compiler_diags.push_back(std::move(d));
  return compiler_diags.back();
}

program_t NULL_program = {nullptr};

program_t* prog;

static short string_idx[0x100];
unsigned char string_tags[0x20];
short freed_string;

/* x_ptr is different inside nested functions */
unsigned short *type_of_locals, *type_of_locals_ptr;
local_info_t *locals, *locals_ptr;

int locals_size = 0;
int type_of_locals_size = 0;
int current_number_of_locals = 0;
int max_num_locals = 0;

/* This function has strput() semantics; see comments in simulate.c */
char* get_two_types(char* where, char* end, int type1, int type2) {
  where = strput(where, end, "( ");
  where = get_type_name(where, end, type1);
  where = strput(where, end, "vs ");
  where = get_type_name(where, end, type2);
  where = strput(where, end, ")");

  return where;
}

void init_locals() {
  auto max_local_variables = CFG_INT(__MAX_LOCAL_VARIABLES__);

  type_of_locals = reinterpret_cast<unsigned short*>(
      DCALLOC(max_local_variables, sizeof(unsigned short), TAG_LOCALS, "init_locals:1"));
  locals = reinterpret_cast<local_info_t*>(
      DCALLOC(max_local_variables, sizeof(local_info_t), TAG_LOCALS, "init_locals:2"));
  type_of_locals_ptr = type_of_locals;
  locals_ptr = locals;
  locals_size = type_of_locals_size = max_local_variables;
  current_number_of_locals = max_num_locals = 0;
}

void free_all_local_names(int flag) {
  int i;

  for (i = 0; i < current_number_of_locals; i++) {
    if (flag && (type_of_locals_ptr[locals_ptr[i].runtime_index] & LOCAL_MOD_UNUSED)) {
      yywarn("Unused local variable '%s'", locals_ptr[i].ihe->name);
    }
    locals_ptr[i].ihe->sem_value--;
    locals_ptr[i].ihe->dn.local_num = -1;
  }
  current_number_of_locals = 0;
  max_num_locals = 0;
  symbol_record(OP_SYMBOL_FREE, current_file, current_line, "");
}

void deactivate_current_locals() {
  int i;

  for (i = 0; i < current_number_of_locals; i++) {
    locals_ptr[i].ihe->dn.local_num = -1;
  }
}

void reactivate_current_locals() {
  int i;

  for (i = 0; i < current_number_of_locals; i++) {
    locals_ptr[i].ihe->dn.local_num = locals_ptr[i].runtime_index;
    locals_ptr[i].ihe->sem_value++;
  }
}

void clean_up_locals() {
  int offset;

  offset = locals_ptr + current_number_of_locals - locals;
  while (offset--) {
    locals[offset].ihe->sem_value--;
    locals[offset].ihe->dn.local_num = -1;
  }
  current_number_of_locals = 0;
  max_num_locals = 0;
  locals_ptr = locals;
  type_of_locals_ptr = type_of_locals;
}

void pop_n_locals(int num) {
  int lcur_start;
  int ltype_start, i1;

  DEBUG_CHECK(num < 0, "pop_n_locals called with num < 0");
  if (num == 0) {
    return;
  }
  symbol_record(OP_SYMBOL_POP, current_file, current_line, std::to_string(num).c_str());
  lcur_start = current_number_of_locals -= num;
  ltype_start = locals_ptr[lcur_start].runtime_index;

  i1 = num;
  while (i1--) {
    if (type_of_locals_ptr[ltype_start] & LOCAL_MOD_UNUSED) {
      yywarn("Unused local variable '%s'", locals_ptr[lcur_start].ihe->name);
    }
    locals_ptr[lcur_start].ihe->sem_value--;
    locals_ptr[lcur_start].ihe->dn.local_num = -1;
    ++lcur_start;
    ++ltype_start;
  }
}

int add_local_name(const char* str, int type, parse_node_t* optional_default_arg_value) {
  auto max_local_variables = CFG_INT(__MAX_LOCAL_VARIABLES__);

  if (max_num_locals == max_local_variables) {
    yyerror("Too many local variables");
    return 0;
  }

  ident_hash_elem_t* ihe;
  symbol_record(OP_SYMBOL_NEW, current_file, current_line, str);
  ihe = find_or_add_ident(str, FOA_NEEDS_MALLOC);
  type_of_locals_ptr[max_num_locals] = type;
  auto idx = current_number_of_locals++;
  locals_ptr[idx].ihe = ihe;
  locals_ptr[idx].funcptr_default = optional_default_arg_value;
  locals_ptr[idx].runtime_index = max_num_locals;
  if (ihe->dn.local_num == -1) {
    ihe->sem_value++;
  }
  return ihe->dn.local_num = max_num_locals++;
}

void reallocate_locals() {
  auto max_local_variables = CFG_INT(__MAX_LOCAL_VARIABLES__);

  int offset;
  offset = type_of_locals_ptr - type_of_locals;
  type_of_locals = RESIZE(type_of_locals, type_of_locals_size += max_local_variables,
                          unsigned short, TAG_LOCALS, "reallocate_locals:1");
  type_of_locals_ptr = type_of_locals + offset;
  offset = locals_ptr - locals;
  locals = RESIZE(locals, locals_size, local_info_t, TAG_LOCALS, "reallocate_locals:2");
  locals_ptr = locals + offset;
}

/* Fix a inherited class type for the current program */
static void fix_class_type(int* t, const program_t* from) {
  ident_hash_elem_t* ihe;
  int num;

  if ((*t) & TYPE_MOD_CLASS) {
    ihe = lookup_ident(from->strings[from->classes[(*t) & CLASS_NUM_MASK].classname]);
    if (ihe && ((num = ihe->dn.class_num) != -1)) {
      (*t) = (*t & ~CLASS_NUM_MASK) | num;
    } else {
      if (!num_parse_error) {
        fatal("Cannot find class %s (%s,%d).\n",
              from->strings[from->classes[(*t) & CLASS_NUM_MASK].classname], current_file,
              current_line);
      }
    }
  }
}

/*
 * Copy all variable names from the object that is inherited from.
 * It is very important that they are stored in the same order with the
 * same index.
 */
void copy_variables(program_t* from, int type) {
  int i;

  for (i = 0; i < from->num_inherited; i++) {
    int t = DECL_MODIFY(type, from->inherit[i].type_mod);

    /* 'private' vars become 'hidden' */
    if (from->inherit[i].type_mod & DECL_PRIVATE) {
      t = (t & ~DECL_PRIVATE) | DECL_HIDDEN;
    }

    t &= ~FUNC_VARARGS; /* for 'varargs inherit' */

    copy_variables(from->inherit[i].prog, t);
  }
  for (i = 0; i < from->num_variables_defined; i++) {
    int t = DECL_MODIFY(from->variable_types[i], type);

    /* 'private' vars become 'hidden' */
    if (from->variable_types[i] & DECL_PRIVATE) {
      t = (t & ~DECL_PRIVATE) | DECL_HIDDEN;
    }

    t &= ~FUNC_VARARGS; /* for 'varargs inherit' */

    /* Handle classes */
    fix_class_type(&t, from);

    define_variable(from->variable_table[i], t);
  }
}

static int add_new_function_entry() {
  int index = mem_block[A_FUNCTION_DEFS].current_size / sizeof(compiler_temp_t);
  allocate_in_mem_block(A_FUNCTION_DEFS, sizeof(compiler_temp_t));

  return index;
}

/* copy a function verbatim into this object, and possibly add it to the
   list of functions in this object, as well
 */

/* This is the first time the function occurs in the current program
   and it happened by copying from program prog
   Because program prog is assumed to be dealt with correctly,
   we know if this _was_ an aliased function, it must point to
   to the correct definition already in prog
   If this is not the first time it occurs, use overload_function
   Otherwise, consistency cannot be guaranteed. The resulting
   first time copy will never be aliased, as it's presence will
   be a magic marker for other similar functions that will get
   removed. - Sym
 */

static void copy_new_function(program_t* prog, int index, program_t* defprog, int defindex,
                              int typemod) {
  ident_hash_elem_t* ihe;

  int where = add_new_function_entry();
  int f = prog->function_flags[index];

  if (f & FUNC_ALIAS) {
    f = FUNC_ALIAS | prog->function_flags[f & ~FUNC_ALIAS];
  }

  /* 'private' functions become 'hidden' */
  if (f & DECL_PRIVATE) {
    f = (f & ~DECL_PRIVATE) | DECL_HIDDEN;
  }

  f = DECL_MODIFY(typemod, f) | FUNC_INHERITED;
  f &= ~DECL_NOSAVE;

  FUNCTION_FLAGS(where) = f;

  FUNCTION_TEMP(where)->prog = defprog;
  FUNCTION_TEMP(where)->u.func = defprog->function_table + defindex;
  FUNCTION_TEMP(where)->offset = NUM_INHERITS - 1;
  FUNCTION_TEMP(where)->function_index_offset = index;
  FUNCTION_NEXT(where) = nullptr;

  /* add the identifier */
  ihe = find_or_add_ident(defprog->function_table[defindex].funcname, FOA_GLOBAL_SCOPE);
  ihe->sem_value++; /* we knew it was the first, so dn.function_num = -1 */
  ihe->dn.function_num = where;
}

static int find_class_member(int which, const char* name, unsigned short* type) {
  int i;
  class_def_t* cd;
  class_member_entry_t* cme;

  DEBUG_CHECK(!findstring(name), "name in find_class_member must be a shared string.\n");

  cd = (reinterpret_cast<class_def_t*>(mem_block[A_CLASS_DEF].block)) + which;
  cme = (reinterpret_cast<class_member_entry_t*>(mem_block[A_CLASS_MEMBER].block)) + cd->index;
  for (i = 0; i < cd->size; i++) {
    if (PROG_STRING(cme[i].membername) == name) {
      break;
    }
  }
  if (i == cd->size) {
    if (type) {
      *type = TYPE_ANY;
    }
    return -1;
  } else {
    if (type) {
      *type = cme[i].type;
    }
    return i;
  }
}

int lookup_any_class_member(char* name, unsigned short* type) {
  int ret = lookup_any_class_member_soft(name, type);
  if (ret == -1) {
    yyerror("No class in scope has no member '%s'.", name);
  }
  return ret;
}

// Like lookup_any_class_member() but silent on a miss -- used by the
// dot/arrow member-access rules to decide whether to fall back to dynamic
// mapping-key access (F_MAP_MEMBER) instead of reporting a class-member
// error, for callers where "not a class member" isn't necessarily wrong.
int lookup_any_class_member_soft(const char* name, unsigned short* type) {
  int nc = mem_block[A_CLASS_DEF].current_size / sizeof(class_def_t);
  int i, ret = -1, nret;
  const char* s = findstring(name);

  if (!s) {
    return -1;
  }

  for (i = 0; i < nc; i++) {
    nret = find_class_member(i, s, type);
    if (nret == -1) {
      continue;
    }
    if (ret != -1 && nret != ret) {
      yyerror("More than one class in scope has member '%s'; use a cast to disambiguate.", name);
    }
    ret = nret;
  }

  return ret;
}

int lookup_class_member(int which, const char* name, unsigned short* type) {
  const char* s = findstring(name);
  int ret;

  if (s) {
    ret = find_class_member(which, s, type);
  } else {
    ret = -1;
  }

  if (ret == -1) {
    class_def_t* cd = (reinterpret_cast<class_def_t*>(mem_block[A_CLASS_DEF].block)) + which;
    yyerror("Class '%s' has no member '%s'", PROG_STRING(cd->classname), name);
  }
  return ret;
}

parse_node_t* reorder_class_values(int which, parse_node_t* node) {
  class_def_t* cd;
  parse_node_t** tmp;
  int i;

  cd = (reinterpret_cast<class_def_t*>(mem_block[A_CLASS_DEF].block)) + which;
  tmp = reinterpret_cast<parse_node_t**>(
      DCALLOC(cd->size, sizeof(parse_node_t*), TAG_COMPILER, "reorder_class_values"));

  for (i = 0; i < cd->size; i++) {
    tmp[i] = nullptr;
  }

  while (node) {
    i = lookup_class_member(which, reinterpret_cast<char*>(node->l.expr), nullptr);
    if (i != -1) {
      if (tmp[i]) {
        yyerror("Redefinition of member '%s' in instantiation of class '%s'",
                reinterpret_cast<char*>(node->l.expr), PROG_STRING(cd->classname));
      } else {
        tmp[i] = node->v.expr;
      }
    }
    /* node->l.expr is an arena c_str (see rule_class_init); bulk-freed. */

    node = node->r.expr;
  }
  i = cd->size;
  node = nullptr;
  while (i--) {
    parse_node_t* newnode;
    if (tmp[i]) {
      CREATE_STATEMENTS(newnode, tmp[i], node);
    } else {
      CREATE_STATEMENTS(newnode, nullptr, node);
      CREATE_NUMBER(newnode->l.expr, 0);
    }
    node = newnode;
  }

  FREE(tmp);
  return node;
}

static void check_class(char* name, const program_t* prog, int idx, int nidx) {
  class_def_t *sd1, *sd2;
  class_member_entry_t *sme1, *sme2;
  int i, n;

  sd1 = &prog->classes[idx];
  sd2 = CLASS(nidx);

  n = sd1->size;
  if (sd1->size != sd2->size) {
    yyerror("Definitions of class '%s' differ in size.", name);
    return;
  }

  sme1 = &prog->class_members[sd1->index];
  sme2 = reinterpret_cast<class_member_entry_t*>(mem_block[A_CLASS_MEMBER].block) + sd2->index;

  for (i = 0; i < n; i++) {
    int newtype;

    newtype = sme1[i].type;

    fix_class_type(&newtype, prog);

    if (sme2[i].type != newtype ||
        prog->strings[sme1[i].membername] != PROG_STRING(sme2[i].membername)) {
      yyerror("Definitions of class '%s' disagree.", name);
      return;
    }
  }
}

void copy_structures(const program_t* prog) {
  class_def_t* sd;
  class_member_entry_t* sme;
  ident_hash_elem_t* ihe;
  char* str;
  int sm_off = mem_block[A_CLASS_MEMBER].current_size / sizeof(class_member_entry_t);
  int sd_off = mem_block[A_CLASS_DEF].current_size / sizeof(class_def_t);
  int i, j, offset;

  if (prog->num_classes + sd_off > CLASS_NUM_MASK) {
    yyerror("Too many classes, max is %d.\n", CLASS_NUM_MASK - 1);
    return;
  }

  for (i = 0; i < prog->num_classes; i++) {
    str = prog->strings[prog->classes[i].classname];
    ihe = find_or_add_ident(str, FOA_GLOBAL_SCOPE);
    if (ihe->dn.class_num == -1) {
      ihe->dn.class_num = sd_off++;
      ihe->sem_value++;
    } else {
      check_class(str, prog, i, ihe->dn.class_num);
      continue;
    }
    sd = reinterpret_cast<class_def_t*>(allocate_in_mem_block(A_CLASS_DEF, sizeof(class_def_t)));
    sd->classname = store_prog_string(str);
    sd->size = prog->classes[i].size;
    sd->index = sm_off;
    sm_off += sd->size;
    sme = reinterpret_cast<class_member_entry_t*>(
        allocate_in_mem_block(A_CLASS_MEMBER, sizeof(class_member_entry_t) * sd->size));
    offset = prog->classes[i].index;
    for (j = 0; j < sd->size; j++) {
      int oldtype = prog->class_members[offset + j].type;

      fix_class_type(&oldtype, prog);
      sme[j].type = oldtype;
      sme[j].membername =
          store_prog_string(prog->strings[prog->class_members[offset + j].membername]);
    }
  }
}

typedef struct ovlwarn_s {
  struct ovlwarn_s* next;
  const char* func;
  char* warn;
} ovlwarn_t;

ovlwarn_t* overload_warnings = nullptr;

static void remove_overload_warnings(const char* func) {
  ovlwarn_t** p;
  ovlwarn_t* tmp;

  p = &overload_warnings;
  while (*p) {
    if (!func || (*p)->func == func) {
      FREE((*p)->warn);
      tmp = *p;
      *p = (*p)->next;
      FREE(tmp);
    } else {
      p = &(*p)->next;
    }
  }
}

static void show_overload_warnings() {
  ovlwarn_t *p, *next;
  p = overload_warnings;
  while (p) {
    // p->warn is a precomposed message embedding program filenames, which
    // can contain '%'; pass it as an argument, not the format.
    yywarn("%s", p->warn);
    FREE(p->warn);
    next = p->next;
    FREE(p);
    p = next;
  }
  overload_warnings = nullptr;
}

/* Overload the function index with the new definition
   The idea is to choose a function out of all non-aliases
   inherited or at the current level.

   non-alias1 ..   non-aliasK -> Final resolution

   The function at oldindex always has the most recent choice.
   The fact that it is at oldindex saves changing ihe.sem_values,
   among other things.

   The current resolution order is
   1st inh proto < ... < last inh proto <
   1st non-inh proto < .. < last non-inh proto <
   1st inh defined < .. < last inh defined <
   defined
 */
static void overload_function(program_t* prog, int index, program_t* defprog, int defindex,
                              int oldindex, int typemod) {
  int f;
  int oldflags = FUNCTION_FLAGS(oldindex);
  function_t* definition = &defprog->function_table[defindex];
  int newflags = prog->function_flags[index];
  int replace;
  compiler_temp_t* newdef;

  if (newflags & FUNC_ALIAS) {
    newflags = FUNC_ALIAS | prog->function_flags[newflags & ~FUNC_ALIAS];
  }

  /* check that we aren't overloading a nomask function */
  if ((oldflags & DECL_NOMASK) && !((oldflags | newflags) & (FUNC_NO_CODE))) {
    yyerror("Illegal to redefine 'nomask' function '%s'.", definition->funcname);
  }

  /* Try to prevent some confusion re: overloading.
   * Warn them about the behavior of inheriting the same function
   * from two branches.
   *
   * Note that we don't want to scream now, b/c if the function is
   * overloaded later this becomes irrelevant.
   *
   * Note also that this is real spammy if you inherit the same object
   * twice.  Something should be done about that.
   */
  if ((pragmas & PRAGMA_WARNINGS) &&
      !((oldflags | newflags) & (FUNC_NO_CODE | DECL_PRIVATE | DECL_HIDDEN)) &&
      (oldflags & FUNC_INHERITED)) {
    /* don't scream if one is private.  Why not?  Because I said so.
     * private is pretty screwed up anyway.  In the future there
     * won't be such a clash b/c private won't come up the tree.
     * This also give the coder a way to shut the compiler up when
     * you do inherit the same object twice in different branches :)
     */
    if (!(oldflags & (DECL_PRIVATE | DECL_HIDDEN)) && !(newflags & (DECL_PRIVATE | DECL_HIDDEN))) {
      char buf[1024];
      char* end = EndOf(buf);
      char* p;
      ovlwarn_t* ow;
      compiler_temp_t* func_entry = FUNCTION_TEMP(oldindex);
      program_t* prog2 = INHERIT(func_entry->offset)->prog;
      program_t* defprog2;

      defprog2 = func_entry->prog;
      p = strput(buf, end, definition->funcname);
      p = strput(p, end, "() inherited from both /");
      p = strput(p, end, defprog->filename);
      if (prog != defprog) {
        p = strput(p, end, " (via /");
        p = strput(p, end, prog->filename);
        p = strput(p, end, ")");
      }
      p = strput(p, end, " and /");
      p = strput(p, end, defprog2->filename);
      if (prog2 != defprog2) {
        p = strput(p, end, " (via /");
        p = strput(p, end, prog2->filename);
        p = strput(p, end, ")");
      }
      p = strput(p, end, "; using the definition in /");
      p = strput(p, end, prog->filename);
      p = strput(p, end, ".");

      ow = reinterpret_cast<ovlwarn_t*>(
          DMALLOC(sizeof(ovlwarn_t), TAG_COMPILER, "overload warning"));
      ow->next = overload_warnings;
      ow->func = definition->funcname;
      ow->warn = alloc_cstring(buf, "overload warning");
      overload_warnings = ow;
    }
  }

  /* A new function also has to be inserted, since this spot will be
   * used when this function is called in an object inheriting us.  Point
   * it at the overloaded function.
   * No need to set much here because epilog() will fixup this entry
   * later.
   */

  newdef = reinterpret_cast<compiler_temp_t*>(
      DMALLOC(sizeof(compiler_temp_t), TAG_COMPILER, "overload_function"));

  /* The resolution order is given in above comments */
  if (oldflags & FUNC_ALIAS) {
    replace = 1;
  } else if (newflags & FUNC_ALIAS) {
    replace = 0;
  } else if (!(newflags & (FUNC_PROTOTYPE | FUNC_UNDEFINED))) {
    if (oldflags & (FUNC_PROTOTYPE | FUNC_INHERITED | FUNC_UNDEFINED)) {
      replace = 1;
    } else {
      replace = 0;
    }
  } else {
    if ((oldflags & (FUNC_PROTOTYPE | FUNC_UNDEFINED)) && (oldflags & FUNC_INHERITED)) {
      replace = 1;
    } else {
      replace = 0;
    }
  }

  if (replace) {
    /* We win */
    if (!(oldflags & FUNC_INHERITED)) {
      /* Mark entry for removal */
      FUNC(FUNCTION_TEMP(oldindex)->u.index)->address = ADDRESS_MAX;
    }
    *newdef = *FUNCTION_TEMP(oldindex);
    FUNCTION_TEMP(oldindex)->next = newdef;
    newdef = FUNCTION_TEMP(oldindex);
  } else {
    compiler_temp_t* oldnext = FUNCTION_TEMP(oldindex)->next;
    newdef->next = oldnext;
    FUNCTION_TEMP(oldindex)->next = newdef;
  }

  newdef->offset = NUM_INHERITS - 1;
  newdef->function_index_offset = index;
  newdef->prog = defprog;
  newdef->u.func = defprog->function_table + defindex;

  f = (newflags & FUNC_MASK) | FUNC_INHERITED;

  /* 'private' functions become 'hidden' */
  if (f & DECL_PRIVATE) {
    f = (f & ~DECL_PRIVATE) | DECL_HIDDEN;
  }

  f = DECL_MODIFY(typemod, f);
  f &= ~DECL_NOSAVE;

  newdef->flags = f;
}

/*
 * Copy all function definitions from an inherited object. They are added
 * as undefined, so that they can be redefined by a local definition.
 * If they are not redefined, then they will be updated, so that they
 * point to the inherited definition. See epilog(). Types will be copied
 * at that moment (if available).
 *
 * A call to an inherited function will not be
 * done through this entry (because this entry can be replaced by a new
 * definition). If an function defined by inheritance is called, then one
 * special definition will be made at first call.
 */
int copy_functions(program_t* from, int typemod) {
  int i, initializer = -1, num_functions;
  ident_hash_elem_t* ihe;
  int num;

  num_functions = from->num_functions_defined + from->last_inherited;

  if (from->num_functions_defined &&
      (strcmp(APPLY___INIT, from->function_table[from->num_functions_defined - 1].funcname) == 0)) {
    initializer = --num_functions;
  }

  for (i = 0; i < num_functions; i++) {
    program_t* prog = from;
    int index = i;
    function_t* funp;
    int low, mid, high;

    /* Walk up the inheritance tree to the real definition */
    if (prog->function_flags[index] & FUNC_ALIAS) {
      index = prog->function_flags[index] & ~FUNC_ALIAS;
    }

    while (prog->function_flags[index] & FUNC_INHERITED) {
      low = 0;
      high = prog->num_inherited - 1;

      while (high > low) {
        mid = (low + high + 1) >> 1;
        if (prog->inherit[mid].function_index_offset > index) {
          high = mid - 1;
        } else {
          low = mid;
        }
      }
      index -= prog->inherit[low].function_index_offset;
      prog = prog->inherit[low].prog;
    }

    index -= prog->last_inherited;

    funp = prog->function_table + index;

    ihe = lookup_ident(funp->funcname);
    if (ihe && ((num = ihe->dn.function_num) != -1)) {
      /* The function has already been defined in this object */
      overload_function(from, i, prog, index, num, typemod);
    } else {
      copy_new_function(from, i, prog, index, typemod);
    }
  }
  return initializer;
}

void type_error(const char* str, int type) {
  static char buff[512];
  char* end = EndOf(buff);
  char* p;

  p = strput(buff, end, str);
  p = strput(p, end, ": \"");
  p = get_type_name(p, end, type);
  p = strput(p, end, "\"");
  yyerror(buff);
}

/*
 * Compare two types, and return true if they are compatible.
 */

/* This one really is t1->t2; it isn't symmetric, since int->void isn't allowed.
 */
int compatible_types(int t1, int t2) {
  if (CONFIG_INT(__RC_OLD_TYPE_BEHAVIOR__)) {
    /* The old version effectively was almost always was true */
    return 1;
  }
  t1 &= ~DECL_MODS;
  t2 &= ~DECL_MODS;
  if (t1 == TYPE_ANY || t2 == TYPE_ANY) {
    return 1;
  }
  if ((t1 == (TYPE_ANY | TYPE_MOD_ARRAY) && (t2 & TYPE_MOD_ARRAY))) {
    return 1;
  }
  if ((t2 == (TYPE_ANY | TYPE_MOD_ARRAY) && (t1 & TYPE_MOD_ARRAY))) {
    return 1;
  }
  if (t1 & TYPE_MOD_CLASS) {
    return t1 == t2;
  }
  if (t1 & TYPE_MOD_ARRAY) {
    if (!(t2 & TYPE_MOD_ARRAY)) {
      return 0;
    }
    return t1 == (TYPE_MOD_ARRAY | TYPE_ANY) || t2 == (TYPE_MOD_ARRAY | TYPE_ANY) || (t1 == t2);
  } else if (t2 & TYPE_MOD_ARRAY) {
    return 0;
  }
  if (t1 > 10 || t1 < 0) {
    fatal("compiler.c: unknown type in compatible_types()");
  }
  return compatible[t1] & (1 << t2);
}

/* This one is symmetric.  Used for comparison operators, etc */
int compatible_types2(int t1, int t2) {
  if (CONFIG_INT(__RC_OLD_TYPE_BEHAVIOR__)) {
    /* The old version effectively was almost always was true */
    return 1;
  }
  t1 &= ~DECL_MODS;
  t2 &= ~DECL_MODS;
  if (t1 == TYPE_ANY || t2 == TYPE_ANY) {
    return 1;
  }
  if ((t1 == (TYPE_ANY | TYPE_MOD_ARRAY) && (t2 & TYPE_MOD_ARRAY))) {
    return 1;
  }
  if ((t2 == (TYPE_ANY | TYPE_MOD_ARRAY) && (t1 & TYPE_MOD_ARRAY))) {
    return 1;
  }
  if (t1 & TYPE_MOD_CLASS) {
    return t1 == t2;
  }
  if (t1 & TYPE_MOD_ARRAY) {
    if (!(t2 & TYPE_MOD_ARRAY)) {
      return 0;
    }
    return t1 == (TYPE_MOD_ARRAY | TYPE_ANY) || t2 == (TYPE_MOD_ARRAY | TYPE_ANY) || (t1 == t2);
  } else if (t2 & TYPE_MOD_ARRAY) {
    return 0;
  }
  if (compatible[t1] & (1 << t2)) {
    return 1;
  }
  return compatible[t2] & (1 << t1);
}

/*
 * Patch a function definition of an inherited function, to what it really
 * should be.
 * The name of the function can be one of:
 *    object::name
 *    ::name
 * Where 'object' is the name of the superclass.
 *
 * Note: this function is now only used for resolving :: references
 */
static int find_matching_function(program_t* prog, const char* name, parse_node_t* node) {
  /* Search our function table */
  for (int i = 0; i < prog->num_functions_defined; i++) {
    // rely on the fact that name is a shared string, can simply compare pointers for equality
    if (name == prog->function_table[i].funcname) {
      int ri;
      int flags;
      int type;

      /* Rely on the fact that functions in the table are not inherited
         or aliased */
      /* Non-inherited aliased ones are always removed anyway */
      ri = prog->last_inherited + i;

      flags = prog->function_flags[ri];

      if (flags & (FUNC_UNDEFINED | FUNC_PROTOTYPE)) {
        // Only a prototype here (e.g. from a header this program included);
        // the real definition may live in one of this program's own
        // inherits, so fall through to the inherit search below instead of
        // failing the whole lookup (issues #1051 / #1088).
        break;
      }
      if (flags & DECL_PRIVATE) {
        return -1;
      }

      node->kind = NODE_CALL_2;
      node->v.number = F_CALL_INHERITED;
      node->l.number = ri;
      type = prog->function_table[i].type;
      fix_class_type(&type, prog);
      node->type = type;
      return 1;
    }
  }
  /* Search inherited function tables */
  int res;
  int i = prog->num_inherited;
  while (i--) {
    if ((res = find_matching_function(prog->inherit[i].prog, name, node))) {
      if ((res == -1) || (prog->inherit[i].type_mod & DECL_PRIVATE)) {
        return -1;
      }

      node->l.number += prog->inherit[i].function_index_offset;
      return 1;
    }
  }
  return 0;
}

int arrange_call_inherited(const char* name, parse_node_t* node) {
  inherit_t* ip;
  int num_inherits, super_length;
  const char *super_name, *p, *real_name = name;
  const char* shared_string;
  int ret;
  std::vector<std::string> names;

  if (real_name[0] == ':') {
    super_name = nullptr;
    real_name += 2; /* There will be exactly two ':' */
    super_length = 0;
  } else if ((p = strchr(real_name, ':'))) {
    super_name = name;
    real_name = p + 2;
    super_length = real_name - super_name - 2;
  } else {
    fatal("no : in inherit call ???");
  }

  num_inherits = NUM_INHERITS;
  shared_string = findstring(real_name);
  /* no need to look for it unless its in the shared string table */
  if (!shared_string) {
    yyerror("No such function '%s' defined.", real_name);
    goto invalid;
  }
  ip = reinterpret_cast<inherit_t*>(mem_block[A_INHERITS].block);
  for (; num_inherits > 0; ip++, num_inherits--) {
    int tmp;

    if (super_name) {
      /* Match against the program name minus its real source
       * extension (".lpc" or ".c" -- the old hardcoded "-2" broke
       * every name::fn() super call into an .lpc-compiled parent,
       * caught by compiler/syntax_functions.lpc). */
      int l = SHARED_STRLEN(ip->prog->filename);
      int base = l;
      names.emplace_back(std::string(ip->prog->filename));

      if (l > 4 && strcmp(ip->prog->filename + l - 4, ".lpc") == 0) {
        base = l - 4;
      } else if (l > 2 && strcmp(ip->prog->filename + l - 2, ".c") == 0) {
        base = l - 2;
      }
      if (base < super_length) {
        continue;
      }
      if (strncmp(super_name, ip->prog->filename + base - super_length, super_length) != 0 ||
          !((base == super_length) || (ip->prog->filename[base - super_length - 1] == '/'))) {
        continue;
      }
    }

    if ((tmp = find_matching_function(ip->prog, shared_string, node))) {
      if (tmp == -1 || (ip->type_mod & DECL_PRIVATE)) {
        yyerror("Called function is private.");

        goto invalid;
      }

      ret = node->l.number + ip->function_index_offset;
      node->l.number |= ((ip - reinterpret_cast<inherit_t*>(mem_block[A_INHERITS].block)) << 16);
      return ret;
    }
  }
  if (super_name) {
    yyerror("Unable to find the inherited function '%s' in file '%s'.", real_name,
            std::string(super_name, super_length).c_str());
    for (auto& name : names) {
      yyerror("  Looked at '%s'", name.c_str());
    }
  } else {
    yyerror("Unable to find the inherited function '%s'.", real_name);
  }
invalid:
  node->kind = NODE_CALL_2;
  node->v.number = F_CALL_INHERITED;
  node->l.number = 0;
  node->type = TYPE_ANY;

  return -1;
}

/*
 * Define a new function. Note that this function is called at least twice
 * for all function definitions. First as a prototype, then as the real
 * function. Thus, there are tests to avoid generating error messages more
 * than once by looking at (flags & NAME_PROTOTYPE).
 */
/* Returns an index into A_FUNCTIONS_DEFS.
 */
int define_new_function(const char* name, int num_arg, int num_local, int flags, int type) {
  int oldindex = -1, num = -1, newindex = -1;
  unsigned short argument_start_index;
  ident_hash_elem_t* ihe;
  function_t* funp = nullptr;
  compiler_temp_t* newfunc;

  oldindex = (ihe = lookup_ident(name)) ? ihe->dn.function_num : -1;
  if (oldindex >= 0) {
    int funflags = FUNCTION_FLAGS(oldindex);

    funp = FUNCTION_DEF(oldindex);

    /*
     * The function was already defined. It may be one of several
     * reasons:
     *
     * 1.   There has been a prototype.
     * 2.   There was a function with the same name defined by inheritance.
     * 3.   This function has been called, but not yet defined.
     * 4.   The function is doubly defined.
     * 5.   A "late" prototype has been encountered.
     */
    if (funflags & FUNC_ALIAS) {
      yyerror("Inconsistent aliasing of functions!\n");
      return -1;
    }

    if (!(funflags & (FUNC_INHERITED | FUNC_PROTOTYPE | FUNC_UNDEFINED)) &&
        !(flags & FUNC_PROTOTYPE)) {
      yyerror("Redeclaration of function '%s'.", name);
      return -1;
    }
    /*
     * It was either an undefined but used funtion, or an inherited
     * function. In both cases, we now consider this to be THE new
     * definition. It might also have been a prototype to an already
     * defined function, in which case we simply return.
     *
     * Check arguments only when types are supposed to be tested, and if
     * this function really has been defined already.
     *
     * 'nomask' functions may not be redefined.
     */
    if ((funflags & DECL_NOMASK) && !((flags | funflags) & (FUNC_UNDEFINED | FUNC_PROTOTYPE))) {
      yyerror("Illegal to redefine 'nomask' function '%s'.", name);
    }

    /* only check prototypes for matching.  It shouldn't be required that
       overloading a function must have the same signature */
    if (exact_types && ((flags | funflags) & FUNC_PROTOTYPE)) {
      int funtype, i;

      funtype = funp->type;
      if (FUNCTION_TEMP(oldindex)->prog) {
        fix_class_type(&funtype, FUNCTION_TEMP(oldindex)->prog);
      }
      /* This should be changed to catch two prototypes which disagree */
      if (funtype != TYPE_UNKNOWN) {
        if (funp->num_arg != num_arg && !((flags | funflags) & FUNC_VARARGS)) {
          yywarn("Number of arguments to '%s' disagrees with previous definition.", name);
        }
        if (!(funflags & FUNC_STRICT_TYPES)) {
          yywarn("Called function '%s' not compiled with type testing.", name);
        }

        /* Now check that argument types wasn't changed. */
        if (!compatible_types(type, funtype)) {
          char buff[512];
          char* end = EndOf(buff);
          char* p;

          if (FUNCTION_TEMP(oldindex)->prog) {
            p = strput(buff, end, "Function ");
            p = strput(p, end, name);
            p = strput(p, end, " inherited from '/");
            p = strput(p, end, FUNCTION_TEMP(oldindex)->prog->filename);
            p = strput(p, end, "' does not match ");
          } else {
            if (funflags & FUNC_PROTOTYPE) {
              p = strput(buff, end, "Previous function prototype ");
            } else {
              p = strput(buff, end, "Previous function declaration ");
            }
            p = strput(p, end, "for ");
            p = strput(p, end, name);
            p = strput(p, end, " does not match ");
          }
          p = strput(p, end, "current function in return type ");
          p = get_two_types(p, end, funtype, type);

          // buff embeds a program filename, which can contain '%'; pass it
          // as an argument, not the format string.
          yywarn("%s", buff);
        }

        for (i = 0; i < num_arg; i++) {
          /* FIXME: check arg types here */
        }
      }
    }

    /* If it was yet another prototype, then simply return. */
    /* provided the current winning definition is an undefined */
    /* or prototype itself */

    if ((flags & FUNC_PROTOTYPE) && !(funflags & (FUNC_PROTOTYPE | FUNC_UNDEFINED))) {
      return -1; /* unused for prototypes */
    }

    if (pragmas & PRAGMA_WARNINGS) {
      remove_overload_warnings(funp->funcname);
    }

    /* If there was already a definition at this level (due to a
     * prototype), clear it out.  Don't free the function name, though;
     * the name is the same, and the ident hash table might be counting
     * on it to stay allocated.
     */
    if (FUNCTION_PROG(oldindex) == nullptr) {
      num = FUNCTION_TEMP(oldindex)->u.index;
      funp = FUNC(num);
    } else {
      funp = nullptr;
    }
  }
  if (!funp) {
    num = mem_block[A_FUNCTIONS].current_size / sizeof(function_t);
    funp = reinterpret_cast<function_t*>(allocate_in_mem_block(A_FUNCTIONS, sizeof(function_t)));
    memset(funp->default_args_findex, 0, sizeof(funp->default_args_findex));
    funp->funcname = make_shared_string(name);
    argument_start_index = INDEX_START_NONE;
    add_to_mem_block(A_ARGUMENT_INDEX, (char*)&argument_start_index, sizeof argument_start_index);
  }

  if (oldindex < 0) {
    newindex = add_new_function_entry();
    ihe = find_or_add_ident(funp->funcname, FOA_GLOBAL_SCOPE);
    ihe->sem_value++;
    ihe->dn.function_num = newindex;
    newfunc = FUNCTION_TEMP(newindex);
    newfunc->next = (compiler_temp_t*)nullptr;
  } else {
    newfunc = reinterpret_cast<compiler_temp_t*>(
        DMALLOC(sizeof(compiler_temp_t), TAG_TEMPORARY, "define_new_function"));
    *newfunc = *FUNCTION_TEMP(oldindex);

    /* We are going to rewrite stuff at oldindex */
    FUNCTION_TEMP(oldindex)->next = newfunc;
    newfunc = FUNCTION_TEMP(oldindex);

    newindex = oldindex;
  }

  newfunc->prog = nullptr;
  newfunc->u.index = num;

  if (exact_types) {
    flags |= FUNC_STRICT_TYPES;
  }
  if (!(flags & DECL_ACCESS)) {
    yyerror("No access level for function!\n");
  }
  newfunc->flags = flags;

  funp->num_local = num_local;
  funp->num_arg = num_arg;
  funp->min_arg = num_arg;
  funp->type = type;
  funp->address = 0;
#ifdef PROFILE_FUNCTIONS
  funp->calls = 0L;
  funp->self = 0L;
  funp->children = 0L;
#endif

  if (exact_types && num_arg) {
    int i;

    if (!(flags & FUNC_PROTOTYPE)) {
      for (i = 0; i < current_number_of_locals; i++) {
        if (type_of_locals_ptr[locals_ptr[i].runtime_index] & LOCAL_MOD_UNUSED) {
          yywarn("Unused local variable '%s'", locals_ptr[i].ihe->name);
          type_of_locals_ptr[locals_ptr[i].runtime_index] &= ~LOCAL_MOD_UNUSED;
        }
      }
    }
    *(reinterpret_cast<unsigned short*>(mem_block[A_ARGUMENT_INDEX].block) + num) =
        mem_block[A_ARGUMENT_TYPES].current_size / sizeof(unsigned short);
    add_to_mem_block(A_ARGUMENT_TYPES, (char*)type_of_locals_ptr,
                     num_arg * sizeof(*type_of_locals_ptr));
    if (!CONFIG_INT(__RC_SUPPRESS_ARGUMENT_WARNINGS__)) {
      if (flags & FUNC_PROTOTYPE) {
        int i;

        for (i = 0; i < num_arg; i++) {
          if (*locals_ptr[i].ihe->name) {
            type_of_locals_ptr[i] |= LOCAL_MOD_UNUSED;
          }
        }
      }
    }
  }
  if (flags & FUNC_PROTOTYPE) {
    symbol_record(OP_SYMBOL_FUNC, current_file, current_line, name);
  }

  return newindex;
}

int define_variable(const char* name, int type) {
  variable_t* dummy;
  int n;
  ident_hash_elem_t* ihe;

  n = (mem_block[A_VAR_TEMP].current_size / sizeof(variable_t));

  ihe = find_or_add_ident(name, FOA_GLOBAL_SCOPE);
  if (ihe->dn.global_num == -1) {
    ihe->sem_value++;
    ihe->dn.global_num = n;

    if (n >= CFG_MAX_GLOBAL_VARIABLES) {
      yyerror("Too many global variables");
    }
  } else {
    yywarn("Redeclaration of global variable '%s'.", ihe->name);

    if (VAR_TEMP(ihe->dn.global_num)->type & DECL_NOMASK) {
      yyerror("Illegal to redefine 'nomask' variable '%s'.", name);
    }
    /* Okay, the nasty idiots have two variables of the same name in
       the same object.  This causes headaches for save_object().
       To keep save_object sane, we need to make one static */
    if (!(VAR_TEMP(ihe->dn.global_num)->type & DECL_NOSAVE)) {
      type |= DECL_NOSAVE;
    }

    /* hidden variables don't cause variables that are visible to become
       invisible; we only add them above (in the !hide case) for better
       error messages */
    if (!(type & DECL_HIDDEN)) {
      ihe->dn.global_num = n;
    }
  }

  dummy = reinterpret_cast<variable_t*>(allocate_in_mem_block(A_VAR_TEMP, sizeof(variable_t)));
  dummy->name = name;
  dummy->type = type;

  return n;
}

int define_new_variable(const char* name, int type) {
  int n;
  unsigned short* tp;
  const char** np;

  var_defined = 1;
  name = make_shared_string(name);
  n = define_variable(name, type);
  np = reinterpret_cast<const char**>(allocate_in_mem_block(A_VAR_NAME, sizeof(char*)));
  *np = name;
  tp = reinterpret_cast<unsigned short*>(allocate_in_mem_block(A_VAR_TYPE, sizeof(unsigned short)));
  *tp = type;
  symbol_record(OP_SYMBOL_VAR, current_file, current_line, name);
  return n;
}

parse_node_t* check_refs(int num, parse_node_t* elist, parse_node_t* pn) {
  int tmp = num;

  elist = elist->r.expr;

  while (elist) {
    if (IS_NODE(elist->v.expr, NODE_UNARY_OP_1, F_MAKE_REF)) {
      tmp--;
    }
    elist = elist->r.expr;
  }
  DEBUG_CHECK(tmp < 0, "Oops, found more refs than is possible!\n");
  if (tmp) {
    /* if we didn't find all the refs at the top level of the argument
     * list, then at least one is buried; i.e. something illegal like
     * func(x = ref y), func(ref y + 1), etc
     */
    yyerror("Illegal use of ref");
  }
  if (num) {
    parse_node_t* ret;

    CREATE_UNARY_OP_1(ret, F_KILL_REFS, pn->type, pn, num);
    return ret;
  }
  return pn;
}

/* we know here that x has two modifiers, and hidden isn't one of them */
int decl_fix(int x) {
  int rest = x & ~DECL_ACCESS;

#ifndef SENSIBLE_MODIFIERS
  if (x & DECL_VISIBLE) {
    return rest | DECL_VISIBLE;
  }
#endif
  if (x & DECL_PRIVATE) {
    return rest | DECL_PRIVATE;
  }

  /* only possibility left is 'public protected' */
  return rest | DECL_PROTECTED;
}

const char* compiler_type_names[] = {"unknown", "mixed",   "void",     "void",  "int",   "string",
                                     "object",  "mapping", "function", "float", "buffer"};

/* This routine has the semantics of strput(); see comments in simulate.c */

char* get_type_modifiers(char* where, char* end, int type) {
#ifdef SENSIBLE_MODIFIERS
  if (type & DECL_HIDDEN) {
    where = strput(where, end, "hidden ");
  }
  if (type & DECL_PRIVATE) {
    where = strput(where, end, "private ");
  }
  if (type & DECL_PROTECTED) {
    where = strput(where, end, "protected ");
  }
  if (type & DECL_PUBLIC) {
    where = strput(where, end, "public ");
  }
  if (type & DECL_NOSAVE) {
    where = strput(where, end, "nosave ");
  }
#else
  if (type & DECL_HIDDEN) {
    where = strput(where, end, "hidden ");
  }
  if (type & DECL_VISIBLE) {
    where = strput(where, end, "public ");
  }
  if (type & DECL_PRIVATE) {
    where = strput(where, end, "private ");
  }
  if (type & DECL_PROTECTED) {
    where = strput(where, end, "static ");
  }
#endif
  /* no output for public */
  if (type & DECL_NOMASK) {
    where = strput(where, end, "nomask ");
  }
  if (type & FUNC_VARARGS) {
    where = strput(where, end, "varargs ");
  }

  return where;
}

char* get_type_name(char* where, char* end, int type) {
  int pointer = 0;

  where = get_type_modifiers(where, end, type);
  type &= ~DECL_MODS;
  if (type & TYPE_MOD_ARRAY) {
    pointer = 1;
    type &= ~TYPE_MOD_ARRAY;
  }
  if (type & TYPE_MOD_CLASS) {
    where = strput(where, end, "class ");
    /* we're sometimes called from outside the compiler * /
    if (current_file)
        where = strput(where, end, PROG_STRING(CLASS(type &
    ~TYPE_MOD_CLASS)->name));
        and that just doesn't work */
  } else {
    DEBUG_CHECK(type >= sizeof compiler_type_names / sizeof compiler_type_names[0], "Bad type\n");
    where = strput(where, end, compiler_type_names[type]);
  }
  where = strput(where, end, " ");
#ifdef ARRAY_RESERVED_WORD
  if (pointer) {
    /* use just "array" instead of "mixed array" */
    if (type == TYPE_ANY) {
      where -= strlen(compiler_type_names[type]) + 1;
    }
    where = strput(where, end, "array ");
  }
#else
  if (pointer) {
    where = strput(where, end, "* ");
  }
#endif
  return where;
}

#define STRING_HASH(var, str)                  \
  var = (intptr_t)str ^ ((intptr_t)str >> 16); \
  var = (var ^ (var >> 8)) & 0xff;

short store_prog_string(const char* str) {
  short i, next, *next_tab, *idxp;
  char** p;
  unsigned char hash, mask, *tagp;

  const auto* origin_str = str;

  bool is_new_string = false;
  str = findstring(origin_str);

  if (!str) {
    str = make_shared_string(origin_str);
    is_new_string = true;
  }

  STRING_HASH(hash, str);
  idxp = &string_idx[hash];

  /* string_tags is a big bit-array, so find correct bit */
  mask = 1 << (hash & 7);
  tagp = &string_tags[hash >> 3];

  p = const_cast<char**>(&PROG_STRING(0));
  next_tab = reinterpret_cast<short*>(mem_block[A_STRING_NEXT].block);

  if (*tagp & mask) {
    /* search hash chain to see if it's there */
    for (i = *idxp; i >= 0; i = next_tab[i]) {
      if (p[i] == str) {
        (reinterpret_cast<short*>(mem_block[A_STRING_REFS].block))[i]++;
        return i;
      }
    }
    next = *idxp;
  } else {
    *tagp |= mask;
    next = -1;
  }

  /*
   * New string, add to table
   */

  if (freed_string >= 0) {
    /* reuse freed string */
    int top;

    i = freed_string;

    top = mem_block[A_STRINGS].current_size / sizeof str;
    for (freed_string++; freed_string < top; freed_string++) {
      if (p[freed_string] == nullptr) {
        break;
      }
    }
    if (freed_string >= top) {
      freed_string = -1;
    }
  } else {
    /* grow by one element. */
    allocate_in_mem_block(A_STRINGS, sizeof str);
    allocate_in_mem_block(A_STRING_NEXT, sizeof(short));
    allocate_in_mem_block(A_STRING_REFS, sizeof(short));
    /* test if number of strings isn't too large ? */
    i = mem_block[A_STRINGS].current_size / sizeof str - 1;
  }
  PROG_STRING(i) = str;
  if (!is_new_string) {
    ref_string(str);
  }
  (reinterpret_cast<short*>(mem_block[A_STRING_NEXT].block))[i] = next;
  (reinterpret_cast<short*>(mem_block[A_STRING_REFS].block))[i] = 1;
  *idxp = i;
  return i;
}

void free_prog_string(short num) {
  short i, prv, *next_tab, top, *idxp;
  char **p, *str;
  unsigned char hash, mask;

  top = mem_block[A_STRINGS].current_size / sizeof(char*) - 1;
  if (num < 0 || num > top) {
    yyerror("free_prog_string: index out of range.\n");
    return;
  }
  if (--(reinterpret_cast<short*>(mem_block[A_STRING_REFS].block))[num] >= 1) {
    return;
  }

  p = reinterpret_cast<char**>(mem_block[A_STRINGS].block);
  next_tab = reinterpret_cast<short*>(mem_block[A_STRING_NEXT].block);

  str = p[num];
  STRING_HASH(hash, str);
  idxp = &string_idx[hash];

  for (prv = -1, i = *idxp; i != num; prv = i, i = next_tab[i]) {
    if (i == -1) {
      yyerror("free_prog_string: string not in prog table.\n");
      return;
    }
  }

  if (prv == -1) { /* string is head of list */
    *idxp = next_tab[i];
    if (*idxp == -1) {
      /* clear tag bit since hash chain now empty */
      mask = 1 << (hash & 7);
      string_tags[hash >> 3] &= ~mask;
    }
  } else { /* easy unlink */
    next_tab[prv] = next_tab[i];
  }

  free_string(str); /* important */
  p[i] = nullptr;
  if (i != top) {
    if (i < freed_string || freed_string == -1) {
      freed_string = i;
    }
  } else {
    /* shrink table */
    mem_block[A_STRINGS].current_size -= sizeof str;
    mem_block[A_STRING_REFS].current_size -= sizeof(short);
    mem_block[A_STRING_NEXT].current_size -= sizeof(short);
  }
}

int validate_function_call(int f, parse_node_t* args) {
  function_t* funp = FUNCTION_DEF(f);
  int funflags = FUNCTION_FLAGS(f);
  int num_arg = (args ? args->kind : 0);
  int num_var = 0;
  parse_node_t* pn = args;
  unsigned short* arg_types = nullptr;
  program_t* prog;

  while (pn) {
    if (pn->type & 1) {
      num_var++;
    }
    pn = pn->r.expr;
  }

  /* Make sure it isn't private */
  if (funflags & DECL_HIDDEN) {
    yyerror("Illegal to call inherited private function '%s'", funp->funcname);
  }

  /*
   * Verify that the function has been defined already.
   */
  if (exact_types) {
    if ((funflags & FUNC_UNDEFINED)) {
      yyerror("Function '%s' undefined.", funp->funcname);
    }
    /*
     * Check number of arguments.
     */
    if (!(funflags & FUNC_VARARGS) && (funflags & FUNC_STRICT_TYPES)) {
      if (num_var) {
        yyerror("Illegal to pass a variable number of arguments to non-varargs function '%s'.",
                funp->funcname);
      } else if (funp->num_arg != num_arg && num_arg < funp->min_arg) {
        yyerror("Wrong number of arguments to '%s', expected: %d, minimum: %d, got: %d.",
                funp->funcname, funp->num_arg, funp->min_arg, num_arg);
      }
    }
    /*
     * Check the argument types.
     */
    if ((prog = FUNCTION_PROG(f))) {
      int which = (FUNCTION_TEMP(f)->u.func - prog->function_table);
      if (prog->type_start) {
        int start = prog->type_start[which];
        if (start != INDEX_START_NONE) {
          arg_types = prog->argument_types + start;
        }
      }
    } else {
      int which = FUNCTION_TEMP(f)->u.index;
      int start = *(reinterpret_cast<unsigned short*>(mem_block[A_ARGUMENT_INDEX].block) + which);
      if (start != INDEX_START_NONE) {
        arg_types = reinterpret_cast<unsigned short*>(mem_block[A_ARGUMENT_TYPES].block) + start;
      }
    }

    if (arg_types) {
      int arg, i, tmp;
      parse_node_t* enode = args;
      int fnarg = funp->min_arg;

      if (funflags & FUNC_TRUE_VARARGS) {
        fnarg--;
      }

      for (i = 0; i < fnarg && i < num_arg; i++) {
        if (enode->type & 1) {
          break;
        }
        tmp = enode->v.expr->type;

        arg = arg_types[i];
        if (prog) {
          fix_class_type(&arg, prog);
        }

        if (!compatible_types(tmp, arg)) {
          char buff[256];
          char* end = EndOf(buff);
          char* p;

          p = strput(buff, end, "Bad type for argument ");
          p = strput_int(p, end, i + 1);
          p = strput(p, end, " of ");
          p = strput(p, end, funp->funcname);
          p = strput(p, end, " ");
          p = get_two_types(p, end, arg_types[i], tmp);
          yyerror(buff);
        }
        enode = enode->r.expr;
      }
    }
  }
  return funp->type;
}

parse_node_t* promote_to_float(parse_node_t* node) {
  parse_node_t* expr;
  if (node->kind == NODE_NUMBER) {
    node->kind = NODE_REAL;
    decltype(node->v.real) tmp = node->v.number;
    node->v.real = tmp;
    return node;
  }
  expr = new_node();
  expr->kind = NODE_EFUN;
  expr->v.number = predefs[to_float_efun].token;
  expr->type = TYPE_REAL;
  expr->l.number = 1;
  expr->r.expr = new_node_no_line();
  expr->r.expr->kind = 1;
  expr->r.expr->l.expr = expr->r.expr;
  expr->r.expr->type = 0;
  expr->r.expr->v.expr = node;
  expr->r.expr->r.expr = nullptr;
  return expr;
}

parse_node_t* promote_to_int(parse_node_t* node) {
  parse_node_t* expr;
  if (node->kind == NODE_REAL) {
    node->kind = NODE_NUMBER;
    decltype(node->v.number) tmp = node->v.real;
    node->v.number = tmp;
    return node;
  }
  expr = new_node();
  expr->kind = NODE_EFUN;
  expr->v.number = predefs[to_int_efun].token;
  expr->type = TYPE_NUMBER;
  expr->l.number = 1;
  expr->r.expr = new_node_no_line();
  expr->r.expr->kind = 1;
  expr->r.expr->l.expr = expr->r.expr;
  expr->r.expr->type = 0;
  expr->r.expr->v.expr = node;
  expr->r.expr->r.expr = nullptr;
  return expr;
}

/* Wrap node in a to_buffer() call: the promotion behind
 * 'buffer b = <string or array of ints 0..255>' and 'b += <same>'. */
parse_node_t* promote_to_buffer(parse_node_t* node) {
  parse_node_t* expr;
  expr = new_node();
  expr->kind = NODE_EFUN;
  expr->v.number = predefs[to_buffer_efun].token;
  expr->type = TYPE_BUFFER;
  expr->l.number = 1;
  expr->r.expr = new_node_no_line();
  expr->r.expr->kind = 1;
  expr->r.expr->l.expr = expr->r.expr;
  expr->r.expr->type = 0;
  expr->r.expr->v.expr = node;
  expr->r.expr->r.expr = nullptr;
  return expr;
}

parse_node_t* add_type_check(parse_node_t* node, int intype) {
  parse_node_t *expr, *expr2;
  int type = 0;

  if (!(pragmas & PRAGMA_STRICT_TYPES)) {
    return node;
  }

  switch (intype & (~DECL_MODS)) {
    case 0:
    case 3:
      // error situation, don't bother
      return node;
    case TYPE_NUMBER:
      type = T_NUMBER;
      break;
    case TYPE_STRING:
      type = T_STRING;
      break;
    case TYPE_OBJECT:
      type = T_OBJECT;
      break;
    case TYPE_MAPPING:
      type = T_MAPPING;
      break;
    case TYPE_FUNCTION:
      type = T_FUNCTION;
      break;
    case TYPE_REAL:
      type = T_REAL;
      break;
    case TYPE_BUFFER:
      type = T_BUFFER;
      break;
    default:
      if (intype & TYPE_MOD_ARRAY) {
        type = T_ARRAY;
      } else {
        type = T_CLASS;
      }
  }

  CREATE_NUMBER(expr2, type);
  CREATE_BINARY_OP(expr, F_TYPE_CHECK, intype, node, expr2);
  return expr;
}

parse_node_t* do_promotions(parse_node_t* node, int type) {
  if (type == TYPE_REAL) {
    if (node->type == TYPE_NUMBER || node->kind == NODE_NUMBER) {
      return promote_to_float(node);
    }
  }
  if (type == TYPE_NUMBER && node->type == TYPE_REAL) {
    return promote_to_int(node);
  }
  if (type == TYPE_BUFFER &&
      (node->type == TYPE_STRING || (node->type & TYPE_MOD_ARRAY))) {
    return promote_to_buffer(node);
  }
  if (type != TYPE_ANY && type != node->type) {
    return add_type_check(node, type);
  }

  return node;
}

/* Take a NODE_CALL, and discard the call, preserving only the args with
   side effects */
parse_node_t* throw_away_call(parse_node_t* pn) {
  parse_node_t* enode;
  parse_node_t* ret = nullptr;
  parse_node_t* arg;

  enode = pn->r.expr;
  while (enode) {
    arg = insert_pop_value(enode->v.expr);
    if (arg) {
      /* woops.  Don't lose the side effect. */
      if (ret) {
        parse_node_t* tmp;
        CREATE_STATEMENTS(tmp, ret, arg);
        ret = tmp;
      } else {
        ret = arg;
      }
    }
    enode = enode->r.expr;
  }
  return ret;
}

parse_node_t* throw_away_mapping(parse_node_t* pn) {
  parse_node_t* enode;
  parse_node_t* ret = nullptr;
  parse_node_t* arg;

  enode = pn->r.expr;
  while (enode) {
    arg = insert_pop_value(enode->v.expr->l.expr);
    if (arg) {
      /* woops.  Don't lose the side effect. */
      if (ret) {
        parse_node_t* tmp;
        CREATE_STATEMENTS(tmp, ret, arg);
        ret = tmp;
      } else {
        ret = arg;
      }
    }
    arg = insert_pop_value(enode->v.expr->r.expr);
    if (arg) {
      /* woops.  Don't lose the side effect. */
      if (ret) {
        parse_node_t* tmp;
        CREATE_STATEMENTS(tmp, ret, arg);
        ret = tmp;
      } else {
        ret = arg;
      }
    }
    enode = enode->r.expr;
  }
  return ret;
}

parse_node_t* validate_efun_call(int f, parse_node_t* args) {
  int num = args->v.number;
  int min_arg, max_arg, def, *argp;
  int num_var = 0;
  parse_node_t* pn = args->r.expr;

  while (pn) {
    if (pn->type & 1) {
      num_var++;
    }
    pn = pn->r.expr;
  }

  if (f != -1) {
    /* should this move out of here? */
    switch (predefs[f].token) {
#ifdef F_SIZEOF
      case F_SIZEOF:
        /* Obscene crap like: sizeof( ({ 1, i++, x + 1, foo() }) )
         *                    -> i++, foo(), 4
         */
        if (!pn && num == 1 && IS_NODE(args->r.expr->v.expr, NODE_CALL, F_AGGREGATE)) {
          parse_node_t *repl, *ret, *node;

          CREATE_NUMBER(node, args->r.expr->v.expr->l.number);
          ret = throw_away_call(args->r.expr->v.expr);
          if (ret) {
            CREATE_TWO_VALUES(repl, TYPE_NUMBER, ret, node);
            return repl;
          } else {
            return node;
          }
        }
#endif
    }

    min_arg = predefs[f].min_args;
    max_arg = predefs[f].max_args;

    def = predefs[f].Default;
    if (!num_var && def != DEFAULT_NONE && num == min_arg - 1) {
      parse_node_t* tmp;
      tmp = new_node_no_line();
      tmp->r.expr = nullptr;
      tmp->type = 0;
      args->l.expr->r.expr = tmp;
      if (def == DEFAULT_THIS_OBJECT) {
        tmp->v.expr = new_node_no_line();
        tmp->v.expr->kind = NODE_EFUN;
        tmp->v.expr->v.number = predefs[this_efun].token;
        tmp->v.expr->l.number = 0;
        tmp->v.expr->type = TYPE_ANY;
        tmp->v.expr->r.expr = nullptr;
      } else {
        CREATE_NUMBER(tmp->v.expr, def);
      }
      args->v.number++;
      num++;
    } else if (num_var && max_arg != -1) {
      yyerror("Illegal to pass variable number of arguments to non-varargs efun '%s'.",
              predefs[f].word);
      CREATE_ERROR(args);
      return args;
    } else if ((num - num_var) < min_arg) {
      yyerror("Too few arguments to '%s'.", predefs[f].word);
      CREATE_ERROR(args);
      return args;
    } else if (num > max_arg && max_arg != -1) {
      yyerror("Too many arguments to '%s'.", predefs[f].word);
      CREATE_ERROR(args);
      return args;
    }
    if (max_arg != -1 && exact_types) {
      /*
       * Now check all types of arguments to efuns.
       */
      int i, argn, tmp;
      parse_node_t* enode = args;
      argp = &efun_arg_types[predefs[f].arg_index];

      for (argn = 0; argn < num; argn++) {
        enode = enode->r.expr;
        if (enode->type & 1) {
          break;
        }
        /* this can happen for default args */
        if (!enode->v.expr) {
          break;
        }
        tmp = enode->v.expr->type;
        for (i = 0; !compatible_types(tmp, argp[i]) && argp[i] != 0; i++) {
          ;
        }

        if (argp[i] == 0) {
          yyerror("Bad argument %d to efun %s()", argn + 1, predefs[f].word);
        } else {
          /* check for (int) -> (float) promotion */
          if (tmp == TYPE_NUMBER && argp[i] == TYPE_REAL) {
            for (i++; argp[i] && argp[i] != TYPE_NUMBER; i++) {
              ;
            }
            if (!argp[i]) {
              enode->v.expr = promote_to_float(enode->v.expr);
            }
          } else if (tmp == TYPE_REAL && argp[i] == TYPE_NUMBER) {
            for (i++; argp[i] && argp[i] != TYPE_REAL; i++) {
              ;
            }
            if (!argp[i]) {
              enode->v.expr = promote_to_int(enode->v.expr);
            }
          }
        }
        while (argp[i] != 0) {
          i++;
        }
        argp += i + 1;
      }
    }
    args->kind = NODE_EFUN;
    args->l.number = num;
    args->v.number = predefs[f].token;
    args->type = predefs[f].ret_type;
    if (args->type == TYPE_NOVALUE) {
      args->v.number += NOVALUE_USED_FLAG;
      args->type = TYPE_VOID;
    }
  } else {
    CREATE_ERROR(args);
  }
  return args;
}

void yyerror(const char* fmt, ...) {
  static char buf[1024 + 1];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  buf[sizeof(buf) - 1] = '\0';

  extern int num_parse_error;

  function_context.num_parameters = -1;
  if (num_parse_error > 5) {
    lex_fatal = 1;
    return;
  }
  report_compile_diagnostic(capture_diagnostic(/*is_warning=*/false, buf));
#ifdef PACKAGE_MUDLIB_STATS
  if (compiler_vm_context) {
    add_errors_for_file(current_file, 1);
  }
#endif
  num_parse_error++;
}

void yyerror(void* yyscanner, const char* msg) { yyerror("%s", msg); }

void yyerror(struct YYLTYPE* llocp, void* yyscanner, const char* msg) {
  // Location-aware Bison entry: primary attribution deliberately stays
  // current_line-at-report (see 8.3's ground rules); llocp feeds nothing
  // yet beyond what capture_diagnostic already reads live.
  (void)llocp;
  (void)yyscanner;
  yyerror("%s", msg);
}

// Flex's YY_FATAL_ERROR is routed here (see lexer.l's prologue) instead of
// the generated yy_fatal_error()'s stock exit(2): an unrecoverable
// scanner condition (in practice only "token too large, exceeds YYLMAX" --
// a single atomic token over 64KB -- or an allocation failure) must abort
// the COMPILE, not the driver. Records the diagnostic, then unwinds via
// the same error() path every other mid-compile abort uses; without a VM
// (unit-test harnesses, where such tokens don't occur) there is no catch
// context to unwind to, so fall through to fatal().
[[noreturn]] void lpc_lex_fatal(const char* msg) {
  lexerror(msg);
  if (compiler_vm_context) {
    error("Fatal lexer error: %s\n", msg);
  }
  fatal("Fatal lexer error outside any VM context: %s\n", msg);
}

void yywarn(const char* fmt, ...) {
  static char buf[1024 + 1];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  buf[sizeof(buf) - 1] = '\0';

  if (!(pragmas & PRAGMA_WARNINGS)) {
    // The suppressed report's queued context must die with it: a
    // producer queues notes/fix-its BEFORE calling yywarn, and leaving
    // them pending would attach them to the NEXT (unrelated) diagnostic.
    compiler_pending_notes.clear();
    compiler_pending_fixits.clear();
    return;
  }

  report_compile_diagnostic(capture_diagnostic(/*is_warning=*/true, buf));
}

/*
 * Compile an LPC file.
 */
program_t* compile_file_fd(int fd, const char* name, vm_context_t* vm_context) {
  prolog_source_fd = fd;  // consumed-and-cleared by prolog (unwind-safe)
  prolog_source_is_fd = true;
  return compile_file(std::string_view{}, name, vm_context);
}

program_t* compile_file(std::string_view source, const char* name, vm_context_t* vm_context) {
  static int guard = 0;
  program_t* prog;
  extern int func_present;

  /* The parser isn't reentrant.  On a few occasions (compile
   * errors, valid_override) LPC code is called during compilation,
   * causing the possibility of arriving here again.
   */
  if (guard || current_file) {
    // A nested compile_file_fd() already stashed its fd; drop the stash
    // before unwinding, or a later string-source compile would consume
    // the stale flag and read from a closed (possibly reused) fd.
    prolog_source_is_fd = false;
    prolog_source_fd = -1;
    error("Object cannot be loaded during compilation.\n");
  }
  guard = 1;

  // Publish this compile's identity on the one state object for the
  // duration (cleared in the DEFER below).
  g_compile.filename = name;

  // Save all compiler globals to support reentrancy/recursive compile
  vm_context_t* saved_vm_context = compiler_vm_context;
  compiler_vm_context = vm_context;
  int saved_current_line = current_line;
  int saved_current_line_base = current_line_base;
  int saved_current_line_saved = current_line_saved;
  int saved_total_lines = total_lines;
  const char* saved_current_file = current_file;
  int saved_current_file_id = current_file_id;
  int saved_pragmas = pragmas;
  int saved_num_parse_error = num_parse_error;
  int saved_lex_fatal = lex_fatal;

  int saved_context = context;
  int saved_num_refs = num_refs;
  int saved_func_present = func_present;

  mem_block_t saved_mem_block[NUMAREAS];
  memcpy(saved_mem_block, mem_block, sizeof(mem_block));

  parse_node_t* saved_comp_trees[NUMTREES];
  memcpy(saved_comp_trees, comp_trees, sizeof(comp_trees));

  int saved_comp_last_inherited = comp_last_inherited;
  int saved_current_tree = current_tree;
  function_context_t saved_function_context = function_context;
  int saved_exact_types = exact_types;
  int saved_global_modifiers = global_modifiers;
  int saved_current_type = current_type;
  int saved_var_defined = var_defined;

  unsigned short* saved_comp_def_index_map = comp_def_index_map;
  unsigned short* saved_func_index_map = func_index_map;
  unsigned short* saved_prog_flags = prog_flags;
  unsigned short* saved_comp_sorted_funcs = comp_sorted_funcs;

  char* saved_prog_code = prog_code;
  char* saved_prog_code_max = prog_code_max;
  program_t* saved_prog = ::prog;

  short saved_string_idx[0x100];
  memcpy(saved_string_idx, string_idx, sizeof(string_idx));

  unsigned char saved_string_tags[0x20];
  memcpy(saved_string_tags, string_tags, sizeof(string_tags));

  short saved_freed_string = freed_string;

  int saved_current_number_of_locals = current_number_of_locals;
  int saved_max_num_locals = max_num_locals;

  // Save the original pointers and sizes of local variable scratchpads
  unsigned short* saved_type_of_locals = type_of_locals;
  local_info_t* saved_locals = locals;
  int saved_type_of_locals_size = type_of_locals_size;
  int saved_locals_size = locals_size;
  unsigned short* saved_type_of_locals_ptr = type_of_locals_ptr;
  local_info_t* saved_locals_ptr = locals_ptr;

  // Allocate fresh, isolated local variable scratchpads for this compilation level
  auto max_local_variables = CFG_INT(__MAX_LOCAL_VARIABLES__);
  type_of_locals = reinterpret_cast<unsigned short*>(
      DCALLOC(max_local_variables, sizeof(unsigned short), TAG_LOCALS, "compile_file:1"));
  locals = reinterpret_cast<local_info_t*>(
      DCALLOC(max_local_variables, sizeof(local_info_t), TAG_LOCALS, "compile_file:2"));
  type_of_locals_size = max_local_variables;
  locals_size = max_local_variables;
  type_of_locals_ptr = type_of_locals;
  locals_ptr = locals;
  current_number_of_locals = 0;
  max_num_locals = 0;

  // The reentrant scanner, initialized BEFORE the cleanup DEFER below so
  // its destruction (guard declared first => runs last) happens AFTER an
  // exception unwind's clean_parser() -> lpc_lex_teardown_active() has
  // deleted the flex buffers -- teardown must never touch a destroyed
  // scanner.
  compiler_context_t lex_ctx;
  void* lex_scanner = nullptr;
  yylex_init_extra(&lex_ctx, &lex_scanner);
  DEFER {
    lpc_lex_scanner_destroyed(lex_scanner);
    yylex_destroy(lex_scanner);
  };

  {
    // make sure we use the C locale during parsing
    auto* current_locale = setlocale(LC_ALL, "C");
    DEFER {
      setlocale(LC_ALL, current_locale);

      // Clean up parser if compile aborted via C++ Exception
      if (std::uncaught_exceptions() > 0) {
        clean_parser();
      }

      // Free the isolated scratchpads allocated for this level
      FREE(type_of_locals);
      FREE(locals);

      // Restore all compiler globals on exit/exception unwinding
      current_line = saved_current_line;
      current_line_base = saved_current_line_base;
      current_line_saved = saved_current_line_saved;
      total_lines = saved_total_lines;
      current_file = saved_current_file;
      current_file_id = saved_current_file_id;
      pragmas = saved_pragmas;
      num_parse_error = saved_num_parse_error;
      lex_fatal = saved_lex_fatal;

      context = saved_context;
      num_refs = saved_num_refs;
      func_present = saved_func_present;

      memcpy(mem_block, saved_mem_block, sizeof(mem_block));
      memcpy(comp_trees, saved_comp_trees, sizeof(comp_trees));

      comp_last_inherited = saved_comp_last_inherited;
      current_tree = saved_current_tree;
      function_context = saved_function_context;
      exact_types = saved_exact_types;
      global_modifiers = saved_global_modifiers;
      current_type = saved_current_type;
      var_defined = saved_var_defined;

      comp_def_index_map = saved_comp_def_index_map;
      func_index_map = saved_func_index_map;
      prog_flags = saved_prog_flags;
      comp_sorted_funcs = saved_comp_sorted_funcs;

      prog_code = saved_prog_code;
      prog_code_max = saved_prog_code_max;
      ::prog = saved_prog;

      memcpy(string_idx, saved_string_idx, sizeof(string_idx));
      memcpy(string_tags, saved_string_tags, sizeof(string_tags));
      freed_string = saved_freed_string;

      current_number_of_locals = saved_current_number_of_locals;
      max_num_locals = saved_max_num_locals;

      type_of_locals = saved_type_of_locals;
      locals = saved_locals;
      type_of_locals_size = saved_type_of_locals_size;
      locals_size = saved_locals_size;
      type_of_locals_ptr = saved_type_of_locals_ptr;
      locals_ptr = saved_locals_ptr;

      compiler_vm_context = saved_vm_context;
      g_compile.filename = nullptr;
      guard = 0;
    };

    symbol_start(name);
    bool loaded = prolog(source, name, lex_scanner);
    if (g_compile.opt_no_optimize) {
      pragmas &= ~PRAGMA_OPTIMIZE;  // pre-optimization staged output
    }
    func_present = 0;

    // Drive the grammar via Bison's push-parser API rather than a
    // generated yyparse()/yypull_parse() pull loop -- grammar.y is push-only
    // (%define api.push-pull push) so those aren't even generated anymore.
    // For a whole-file compile this loop always runs to completion in one
    // call (every token is available immediately), but it's the same
    // token-at-a-time next()+yypush_parse() shape a future incremental/REPL
    // driver needs, just without ever pausing between tokens.
    auto pstate = std::unique_ptr<yypstate, void (*)(yypstate*)>(yypstate_new(), yypstate_delete);
    if (!pstate) {
      yyerror(lex_scanner, "memory exhausted");
    } else {
      int push_status;
      do {
        if (!loaded) break;  // source failed to load: no buffer to scan
        YYSTYPE yylval;
        YYLTYPE yylloc;
        int token = yylex(&yylval, &yylloc, lex_scanner);
        push_status = yypush_parse(pstate.get(), token, &yylval, &yylloc, lex_scanner);
      } while (push_status == YYPUSH_MORE);
    }

    symbol_end();
    prog = epilog();
  }

  return prog;
}

int get_id_number() {
  static int current_id_number = 1;
  return current_id_number++;
}

static void copy_in(int which, char** start) {
  char* block;
  int size;

  size = mem_block[which].current_size;
  if (!size) {
    return;
  }

  block = mem_block[which].block;
  memcpy(*start, block, size);

  *start += align(size);
}

static int compare_funcs(const void* x, const void* y) {
  const char* n1 = FUNC(*(unsigned short*)x)->funcname;
  const char* n2 = FUNC(*(unsigned short*)y)->funcname;
  int sp1, sp2;

  /* make sure #global_init# stays last; also shuffle empty entries to
   * the end so we can delete them easily.
   */
  if (FUNC(*(unsigned short*)x)->address == ADDRESS_MAX) {
    sp1 = 3;
  } else if (strcmp(n1, APPLY___INIT) == 0) {
    sp1 = 2;
  } else if (n1[0] == APPLY___INIT_SPECIAL_CHAR) {
    sp1 = 1;
  } else {
    sp1 = 0;
  }

  if (FUNC(*(unsigned short*)y)->address == ADDRESS_MAX) {
    sp2 = 3;
  } else if (strcmp(n2, APPLY___INIT) == 0) {
    sp2 = 2;
  } else if (n2[0] == APPLY___INIT_SPECIAL_CHAR) {
    sp2 = 1;
  } else {
    sp2 = 0;
  }

  if (sp1 < sp2) {
    return -1;
  }
  if (sp1 > sp2) {
    return 1;
  }

  if (n1 < n2) {
    return -1;
  }
  if (n1 > n2) {
    return 1;
  }

  return 0;
}

static void handle_functions() {
  int num_func, total_func;
  int i;
  compiler_temp_t* cur_def;
  int new_index, num_def;
  int final_index;
  inherit_t* inheritp;

  /* Pass one: Sort the compiler functions first                        */

  num_func = total_func = mem_block[A_FUNCTIONS].current_size / sizeof(function_t);
  if (num_func) {
    func_index_map = reinterpret_cast<unsigned short*>(
        DCALLOC(num_func, sizeof(unsigned short), TAG_TEMPORARY, "handle_functions"));
    comp_sorted_funcs = reinterpret_cast<unsigned short*>(
        DCALLOC(num_func, sizeof(unsigned short), TAG_TEMPORARY, "handle_functions"));

    i = num_func;
    while (i--) {
      func_index_map[i] = i;
    }

    // FIXME: compare_func uses pointer address to sort the index, which is not deterministic.
    qsort(func_index_map, num_func, sizeof(unsigned short), compare_funcs);

    i = num_func;
    while (i--) {
      comp_sorted_funcs[func_index_map[i]] = i;
    }

    while (num_func && FUNC(func_index_map[num_func - 1])->address == ADDRESS_MAX) {
      num_func--;
    }
  }

  if (NUM_INHERITS) {
    program_t* inherited_prog;

    inheritp = INHERIT(NUM_INHERITS - 1);

    inherited_prog = inheritp->prog;

    comp_last_inherited = inheritp->function_index_offset + inherited_prog->last_inherited +
                          inherited_prog->num_functions_defined;

    if (inherited_prog->num_functions_defined &&
        strcmp(
            APPLY___INIT,
            inherited_prog->function_table[inherited_prog->num_functions_defined - 1].funcname) ==
            0) {
      comp_last_inherited--;
    }
  } else {
    comp_last_inherited = 0;
  }

  /* Pass one: We allocate space for the comp_def_index_map             */
  /*           and prog_flags, and fill them out in sequential order    */

  num_def = mem_block[A_FUNCTION_DEFS].current_size / sizeof(compiler_temp_t);
  if (num_def) {
    comp_def_index_map = reinterpret_cast<unsigned short*>(
        DCALLOC(num_def, sizeof(unsigned short), TAG_TEMPORARY, "handle functions"));
    prog_flags = reinterpret_cast<unsigned short*>(DCALLOC(comp_last_inherited + total_func,
                                                           sizeof(unsigned short), TAG_TEMPORARY,
                                                           "handle_functions"));

    for (i = 0; i < num_def; i++) {
      cur_def = FUNCTION_TEMP(i);

      if (cur_def->flags & FUNC_INHERITED) {
        final_index =
            INHERIT(cur_def->offset)->function_index_offset + cur_def->function_index_offset;
      } else {
        final_index = comp_last_inherited + comp_sorted_funcs[cur_def->u.index];
      }
      if (cur_def->flags & FUNC_ALIAS) {
        yyerror("Aliasing difficulties!\n");
        return;
      }

      comp_def_index_map[i] = final_index;
      prog_flags[final_index] = cur_def->flags;

      while ((cur_def = cur_def->next)) {
        if (cur_def->flags & FUNC_INHERITED) {
          new_index =
              INHERIT(cur_def->offset)->function_index_offset + cur_def->function_index_offset;
        } else {
          new_index = comp_last_inherited + comp_sorted_funcs[cur_def->u.index];
        }
        /* We aren't worried about repeating new_indices */
        /* because it's all the same value               */
        /* except the case where new_index is actually final_index */

        if (new_index != final_index) {
          prog_flags[new_index] = FUNC_ALIAS | final_index;
        }
      }
    }
  }
}

/*
 * The program has been compiled. Prepare a 'program_t' to be returned.
 */
static program_t* epilog(void) {
  int size, i, lnsz, lnoff;
  char* p;
  int num_func;
  ident_hash_elem_t* ihe;
  program_t* prog;
  compiler_temp_t *fundefp, *nextdefp;

  deinit_include_path();

  if (num_parse_error > 0 || inherit_file) {
    /* don't print these; they can be wrong, since we didn't parse the
       entire file */
    if (pragmas & PRAGMA_WARNINGS) {
      remove_overload_warnings(nullptr);
    }
    clean_parser();
    end_new_file();
    free_string(current_file);
    current_file = nullptr;
    return nullptr;
  }

  if (pragmas & PRAGMA_WARNINGS) {
    show_overload_warnings();
  }

  /*
   * Define the #global_init# function, but only if there was any code
   * to initialize.
   */
  UPDATE_PROGRAM_SIZE;

  if (comp_trees[TREE_INIT]) {
    parse_node_t *pn, *newnode;
    int fun;
    /* end the __INIT function */
    CREATE_RETURN(pn, nullptr);
    newnode = comp_trees[TREE_INIT];
    CREATE_TWO_VALUES(comp_trees[TREE_INIT], 0, newnode, pn);
    fun = define_new_function(APPLY___INIT, 0, 0, DECL_HIDDEN | FUNC_STRICT_TYPES, TYPE_VOID);
    pn = new_node_no_line();
    pn->kind = NODE_FUNCTION;
    pn->v.number = fun;
    pn->l.number = 0;
    pn->r.expr = comp_trees[TREE_INIT];
    comp_trees[TREE_INIT] = pn;
  }

  /* Do stuff with functions first */
  /* calculate comp_last_inherited also */
  handle_functions();

  /* generate the trees */

  current_tree = TREE_MAIN;
  if (g_compile.opt_dump_ast) {
    printf(";;; AST %s -- TREE_MAIN\n", g_compile.filename != nullptr ? g_compile.filename : "?");
    dump_tree(comp_trees[TREE_MAIN]);
    printf("\n;;; AST -- TREE_INIT\n");
    dump_tree(comp_trees[TREE_INIT]);
    printf("\n");
  }
  generate(comp_trees[TREE_MAIN]);
  // DEBUG:
  // dump_tree(comp_trees[TREE_MAIN]);

  current_tree = TREE_INIT;
  generate(comp_trees[TREE_INIT]);
  // DEBUG:
  // dump_tree(comp_trees[TREE_INIT]);

  current_tree = TREE_MAIN;

  generate_final_program(0);
  UPDATE_PROGRAM_SIZE;

  if (mem_block[0].current_size > ADDRESS_MAX) {
    yyerror("Program too large");
    clean_parser();
    end_new_file();
    free_string(current_file);
    current_file = nullptr;
    return nullptr;
  }

  generate_final_program(1);

  free_tree();

  size = align(sizeof(program_t));

  /* delete argument information if we're not saving it */
  if (!(pragmas & PRAGMA_SAVE_TYPES)) {
    mem_block[A_ARGUMENT_TYPES].current_size = 0;
  }
  if (!(mem_block[A_ARGUMENT_TYPES].current_size)) {
    mem_block[A_ARGUMENT_INDEX].current_size = 0;
  }

  for (i = 0; i < NUMPAREAS; i++) {
    if (i != A_LINENUMBERS && i != A_FILE_INFO && i != A_FUNCTION_DEFS && i != A_FUNCTIONS &&
        i != A_ARGUMENT_INDEX) {
      size += align(mem_block[i].current_size);
    }
  }

  num_func = mem_block[A_FUNCTIONS].current_size / sizeof(function_t);

  while (num_func && FUNC(func_index_map[num_func - 1])->address == ADDRESS_MAX) {
    num_func--;
  }

  size += align((num_func * sizeof(function_t))); /* A_FUNCTIONS */

  /* function flags */
  size += align(((comp_last_inherited + num_func) * sizeof(unsigned short)));

  /* A_ARGUMENT_INDEX */
  if (mem_block[A_ARGUMENT_INDEX].current_size) {
    size += align(num_func * sizeof(unsigned short));
  }

  p = reinterpret_cast<char*>(DMALLOC(size, TAG_PROGRAM, "epilog: 1"));
  prog = new (p) program_t;
  prog->total_size = size;
  prog->ref = 0;
  prog->func_ref = 0;
  ihe = lookup_ident("heart_beat");
  if (ihe && ihe->dn.function_num != -1) {
    prog->heart_beat = comp_def_index_map[ihe->dn.function_num] + 1;
    if (prog_flags && prog_flags[prog->heart_beat - 1] & (FUNC_PROTOTYPE | FUNC_UNDEFINED)) {
      prog->heart_beat = 0;
    }
  } else {
    prog->heart_beat = 0;
  }
  prog->filename = current_file;

  current_file = nullptr;

  total_num_prog_blocks++;
  total_prog_block_size += size;

  /* Format is now:
   * <short total size> <short line_info_offset> <file info> <line info>
   */
  lnoff = 2 + (mem_block[A_FILE_INFO].current_size / sizeof(short));
  lnsz = lnoff * sizeof(short) + mem_block[A_LINENUMBERS].current_size;

  prog->file_info = reinterpret_cast<unsigned short*>(DMALLOC(lnsz, TAG_LINENUMBERS, "epilog"));

  prog->file_info[0] = static_cast<unsigned short>(lnsz);
  prog->file_info[1] = static_cast<unsigned short>(lnoff);

  memcpy((reinterpret_cast<char*>(&prog->file_info[2])), mem_block[A_FILE_INFO].block,
         mem_block[A_FILE_INFO].current_size);

  prog->line_info = reinterpret_cast<unsigned char*>(&prog->file_info[lnoff]);
  memcpy((reinterpret_cast<char*>(&prog->file_info[lnoff])), mem_block[A_LINENUMBERS].block,
         mem_block[A_LINENUMBERS].current_size);

  p += align(sizeof(program_t));
  prog->program = p;
  prog->program_size = mem_block[A_PROGRAM].current_size;
  copy_in(A_PROGRAM, &p);

  /* copy the functions in */
  prog->last_inherited = comp_last_inherited;
  prog->num_functions_defined = num_func;

  prog->function_table = reinterpret_cast<function_t*>(p);
  for (i = 0; i < num_func; i++) {
    prog->function_table[i] = *FUNC(func_index_map[i]);
    // debug_message("Function table %d: %s\n", i, prog->function_table[i].funcname);
  }

  // Entries sorted past num_func were excluded from the program table
  // (address == ADDRESS_MAX: local prototypes/undefineds superseded by an
  // inherited definition). Nothing takes over their funcname refs, so
  // release them here or each such entry leaks one shared-string ref per
  // compile.
  {
    int const total_func = mem_block[A_FUNCTIONS].current_size / sizeof(function_t);
    for (i = num_func; i < total_func; i++) {
      function_t* dead = FUNC(func_index_map[i]);
      if (dead->funcname) {
        free_string(dead->funcname);
        dead->funcname = nullptr;
      }
    }
  }

  // Fixup the default argument function index
  for (int i = 0; i < num_func; i++) {
    auto* func = &prog->function_table[i];
    constexpr auto default_args_limit =
        sizeof(func->default_args_findex) / sizeof(func->default_args_findex[0]);
    if (func->min_arg != func->num_arg) {
      // debug_message("Handling default arguments for %d: %s\n", i, func->funcname);
      for (int j = 0; j < default_args_limit; j++) {
        auto findex = func->default_args_findex[j];
        if (findex != 0) {
          func->default_args_findex[j] = comp_sorted_funcs[findex];
          // debug_message("Default argument %d of function %s was %d, now is %d\n", j,
          // func->funcname, findex, func->default_args_findex[j]);
          if (prog->function_table[(func->default_args_findex[j])].funcname[0] !=
              APPLY___INIT_SPECIAL_CHAR) {
            func->default_args_findex[j] = 0;  // attempt to continue;
            DEBUG_FATAL("Bad new default argument index calculated\n");
          }
        }
      }
    }
  }

  // Guard on the pointer, not num_func: handle_functions() decrements
  // num_func past trailing ADDRESS_MAX entries (locals superseded by an
  // inherited definition), so a program whose entries are ALL superseded
  // reaches here with num_func == 0 but the array live.
  if (comp_sorted_funcs) {
    FREE((char*)comp_sorted_funcs);
    comp_sorted_funcs = nullptr;
  }

  p += align(sizeof(function_t) * num_func);

  prog->function_flags = reinterpret_cast<unsigned short*>(p);
  if (prog_flags) {
    memcpy(p, prog_flags, (comp_last_inherited + num_func) * sizeof(unsigned short));
    FREE((char*)prog_flags);
  }
  p += align(sizeof(unsigned short) * (comp_last_inherited + num_func));

  if (mem_block[A_ARGUMENT_INDEX].current_size) {
    unsigned short* dest;

    prog->argument_types = reinterpret_cast<unsigned short*>(p);
    copy_in(A_ARGUMENT_TYPES, &p);

    dest = prog->type_start = reinterpret_cast<unsigned short*>(p);
    i = num_func;
    while (i--) {
      dest[i] = *(reinterpret_cast<unsigned short*>(mem_block[A_ARGUMENT_INDEX].block) +
                  func_index_map[i]);
    }
    p += align(num_func * sizeof(unsigned short));
  } else {
    prog->argument_types = nullptr;
    prog->type_start = nullptr;
  }

  prog->classes = reinterpret_cast<class_def_t*>(p);
  prog->num_classes = mem_block[A_CLASS_DEF].current_size / sizeof(class_def_t);
  copy_in(A_CLASS_DEF, &p);

  prog->class_members = reinterpret_cast<class_member_entry_t*>(p);
  copy_in(A_CLASS_MEMBER, &p);

  prog->strings = reinterpret_cast<char**>(p);
  prog->num_strings = mem_block[A_STRINGS].current_size / sizeof(char*);
  copy_in(A_STRINGS, &p);

  prog->num_variables_defined = mem_block[A_VAR_NAME].current_size / sizeof(char*);
  prog->num_variables_total = mem_block[A_VAR_TEMP].current_size / sizeof(variable_t);

  prog->variable_table = reinterpret_cast<char**>(p);
  copy_in(A_VAR_NAME, &p);
  prog->variable_types = reinterpret_cast<unsigned short*>(p);
  copy_in(A_VAR_TYPE, &p);

  prog->num_inherited = mem_block[A_INHERITS].current_size / sizeof(inherit_t);
  if (prog->num_inherited) {
    prog->inherit = reinterpret_cast<inherit_t*>(p);
    copy_in(A_INHERITS, &p);
  } else {
    prog->inherit = nullptr;
  }

  prog->apply_lookup_table.reset(nullptr);

#ifdef DEBUG
  if (p - reinterpret_cast<char*>(prog) != size) {
    debug_message("Program size miscalculated for /%s.\n", prog->filename);
    debug_message("is: %ld, expected: %d\n", p - reinterpret_cast<char*>(prog), size);
  }
#endif

  for (i = 0; i < mem_block[A_FUNCTION_DEFS].current_size / sizeof(*fundefp); i++) {
    fundefp = FUNCTION_TEMP(i)->next;
    while (fundefp) {
      nextdefp = fundefp->next;
      FREE((char*)fundefp);
      fundefp = nextdefp;
    }
  }

  // Buffer teardown BEFORE the mem_block frees, mirroring clean_parser:
  // pops perform include accounting that writes into mem_block. (On this
  // success path all buffers already drained at EOF -- ordering kept
  // symmetric as defense in depth.)
  lpc_lex_teardown_active();

  for (i = 0; i < NUMAREAS; i++) {
    FREE((char*)mem_block[i].block);
  }
  if (comp_def_index_map) {
    FREE((char*)comp_def_index_map);
  }
  if (func_index_map) {
    FREE((char*)func_index_map);
  }

  /*  marion
      Do referencing here - avoid multiple referencing when an object
      inherits more than one object and one of the inherited is already
      loaded and not the last inherited
  */
  reference_prog(prog, "epilog");
  for (i = 0; static_cast<unsigned>(i) < prog->num_inherited; i++) {
    reference_prog(prog->inherit[i].prog, "inheritance");
  }
  release_tree();
  uninitialize_parser();
  scratch_destroy();
  clean_up_locals();
  free_unused_identifiers();
  end_new_file();

  return prog;
}

/*
 * Initialize the environment that the compiler needs.
 */
static bool prolog(std::string_view source, const char* name, void* scanner) {
  int i;

  function_context.num_parameters = -1;
  num_parse_error = 0;
  global_modifiers = 0;
  var_defined = 0;

  /* Initialize memory blocks where the result of the compilation
   * will be stored.
   */
  for (i = 0; i < NUMAREAS; i++) {
    mem_block[i].block =
        reinterpret_cast<char*>(DMALLOC(START_BLOCK_SIZE, TAG_COMPILER, "prolog: 2"));
    mem_block[i].current_size = 0;
    mem_block[i].max_size = START_BLOCK_SIZE;
  }
  for (i = 0; i < NUMTREES; i++) {
    comp_trees[i] = nullptr;
  }
  prog_flags = nullptr;
  func_index_map = nullptr;
  comp_sorted_funcs = nullptr;
  comp_def_index_map = nullptr;

  memset(string_tags, 0, sizeof(string_tags));
  freed_string = -1;
  initialize_parser();

  current_file = make_shared_string(name);
  current_file_id = add_program_file(name, 1);

  init_include_path();

  /*
   * if we've got a simul_efun object and we're not reloading it, make a copy
   * of its class definitions in the object we're compiling now.
   */
  if (simul_efun_ob && *simul_efun_ob->obname) {
    copy_structures(simul_efun_ob->prog);
  }

  // Consume-and-clear the fd BEFORE loading: an exception unwind from the
  // load must never leave a stale fd flag poisoning the next compile.
  bool use_fd = prolog_source_is_fd;
  int fd = prolog_source_fd;
  prolog_source_is_fd = false;
  prolog_source_fd = -1;
  if (use_fd) {
    if (!start_new_file_fd(fd, scanner)) {
      yyerror(scanner, "could not read source file");
      num_parse_error++;
      return false;
    }
  } else {
    start_new_file(source, scanner);
  }
  return true;
}

/*
 * The program has errors, clean things up.
 */
static void clean_parser() {
  // FIRST: delete leftover flex buffers. An aborted compile (inherit
  // abort / fatal) can end with #include buffers still live, and popping
  // them runs the include accounting (pop_include_state ->
  // save_file_info -> add_to_mem_block), which writes into mem_block --
  // so teardown MUST precede the mem_block frees below. Ordering this
  // after them was a real heap-use-after-free on a live mud (inherit
  // inside an included file).
  lpc_lex_teardown_active();

  int i, n;
  function_t* funp;
  compiler_temp_t *fundefp, *nextdefp;

  /*
   * Free function stuff.
   */
  for (i = 0; i < mem_block[A_FUNCTIONS].current_size / sizeof(function_t); i++) {
    funp = FUNC(i);
    if (funp->funcname) {
      free_string(funp->funcname);
    }
  }

  for (i = 0; i < mem_block[A_FUNCTION_DEFS].current_size / sizeof(compiler_temp_t); i++) {
    fundefp = FUNCTION_TEMP(i)->next;
    while (fundefp) {
      nextdefp = fundefp->next;
      FREE((char*)fundefp);
      fundefp = nextdefp;
    }
  }

  n = mem_block[A_STRINGS].current_size / sizeof(char*);
  for (i = 0; i < n; i++) {
    free_string(*(reinterpret_cast<char**>(mem_block[A_STRINGS].block) + i));
  }
  n = mem_block[A_VAR_NAME].current_size / sizeof(char*);
  for (i = 0; i < n; i++) {
    free_string(*(reinterpret_cast<char**>(mem_block[A_VAR_NAME].block) + i));
  }

  prog = nullptr;
  for (i = 0; i < NUMAREAS; i++) {
    FREE(mem_block[i].block);
  }
  if (comp_def_index_map) {
    FREE((char*)comp_def_index_map);
  }
  if (func_index_map) {
    FREE((char*)func_index_map);
  }
  if (comp_sorted_funcs) {
    FREE((char*)comp_sorted_funcs);
    comp_sorted_funcs = nullptr;
  }
  if (prog_flags) {
    FREE((char*)prog_flags);
  }

  /* don't need the parse trees any more */
  release_tree();
  uninitialize_parser();
  clean_up_locals();
  // (Buffers were torn down at the top of this function; the arena reset
  // must still come after everything that reads arena memory.)
  scratch_destroy();
  free_unused_identifiers();
}

char* the_file_name(const char* name) {
  char* tmp;
  int len;

  len = strlen(name);
  if (len < 3) {
    return string_copy(name, "the_file_name");
  }
  tmp = new_string(len - 1, "the_file_name");
  if (!tmp) {
    return string_copy(name, "the_file_name");
  }
  tmp[0] = '/';
  strncpy(tmp + 1, name, len - 2);
  tmp[len - 1] = '\0';
  return tmp;
}

static int case_compare(const void* c1, const void* c2) {
  /* sort DEFAULT to the end */
  if ((*(parse_node_t**)c1)->kind == NODE_DEFAULT) {
    return -1;
  }
  if ((*(parse_node_t**)c2)->kind == NODE_DEFAULT) {
    return 1;
  }
  return COMPARE_NUMS((*(parse_node_t**)c1)->r.number, (*(parse_node_t**)c2)->r.number);
}

static int string_case_compare(const void* c1, const void* c2) {
  LPC_INT x, y;

  /* sort DEFAULT to the end */
  if ((*(parse_node_t**)c1)->kind == NODE_DEFAULT) {
    return -1;
  }
  if ((*(parse_node_t**)c2)->kind == NODE_DEFAULT) {
    return 1;
  }
  x = (*(parse_node_t**)c1)->r.number;
  y = (*(parse_node_t**)c2)->r.number;
  x = x ? (((POINTER_INT)PROG_STRING(x))) : 0;
  y = y ? (((POINTER_INT)PROG_STRING(y))) : 0;
  return COMPARE_NUMS(x, y);
}

void prepare_cases(parse_node_t* pn, int start) {
  parse_node_t **ce_start, **ce_end, **ce;
  LPC_INT last_key, this_key;
  int end;
  int direct = 1;

  ce_start = reinterpret_cast<parse_node_t**>(&mem_block[A_CASES].block[start]);
  end = mem_block[A_CASES].current_size;
  ce_end = reinterpret_cast<parse_node_t**>(&mem_block[A_CASES].block[end]);

  if (ce_start == ce_end) {
    /* no cases */
    pn->v.expr = nullptr;
    mem_block[A_CASES].current_size = start;
    return;
  }

  if (pn->kind == NODE_SWITCH_STRINGS) {
    qsort(ce_start, ce_end - ce_start, sizeof(parse_node_t*), string_case_compare);
  } else {
    qsort(ce_start, ce_end - ce_start, sizeof(parse_node_t*), case_compare);
  }

  ce = ce_start;
  if ((*ce)->kind == NODE_DEFAULT) {
    if (ce + 1 == ce_end) {
      /* only a default */
      pn->v.expr = *ce;
      (*ce)->l.expr = nullptr;
      mem_block[A_CASES].current_size = start;
      return;
    }
    ce++;
    (*(ce - 1))->l.expr = *ce;
  }
  if ((*ce)->v.expr) {
    last_key = (*ce)->v.expr->r.number;
    direct = 0;
  } else {
    last_key = (*ce)->r.number;
  }
  ce++;
  while (ce < ce_end) {
    this_key = (*ce)->r.number;
    if (pn->kind == NODE_SWITCH_RANGES && this_key <= last_key) {
      char buf[1024];
      char* end = EndOf(buf);
      char* p;
      const char *f1, *f2;
      int fi1, fi2;
      int l1, l2;

      /* make sure line numbers exist for the cases */
      save_file_info(current_file_id, current_line - current_line_saved);
      current_line_saved = current_line;

      translate_absolute_line(
          (*ce)->line, reinterpret_cast<unsigned short*>(mem_block[A_FILE_INFO].block), &fi1, &l1);
      translate_absolute_line((*(ce - 1))->line,
                              reinterpret_cast<unsigned short*>(mem_block[A_FILE_INFO].block), &fi2,
                              &l2);
      f1 = PROG_STRING(fi1 - 1);
      f2 = PROG_STRING(fi2 - 1);

      p = strput(buf, end, "Overlapping cases: ");
      if (f1) {
        p = strput(p, end, f1);
        p = strput(p, end, ":");
      } else {
        p = strput(p, end, "line ");
      }
      p = strput_int(p, end, l1);
      p = strput(p, end, " and ");
      if (f2) {
        p = strput(p, end, f2);
        p = strput(p, end, ":");
      } else {
        p = strput(p, end, "line ");
      }
      p = strput_int(p, end, l2);
      p = strput(p, end, ".");
      yyerror(buf);
    }
    (*(ce - 1))->l.expr = *ce;
    if ((*ce)->v.expr) {
      last_key = (*ce)->v.expr->r.number;
      direct = 0;
    } else {
      if (last_key + 1 != this_key) {
        direct = 0;
      }
      last_key = this_key;
    }
    ce++;
  }
  (*(ce_end - 1))->l.expr = nullptr;
  if (direct && pn->kind == NODE_SWITCH_NUMBERS) {
    pn->kind = NODE_SWITCH_DIRECT;
  }
  pn->v.expr = *(ce_start);
  mem_block[A_CASES].current_size = start;
}

void save_file_info(int file_id, int lines) {
  short fi[2];

  fi[0] = lines;
  fi[1] = file_id;
  add_to_mem_block(A_FILE_INFO, (char*)&fi[0], sizeof(fi));
}

int add_program_file(const char* name, int top) {
  if (!top) {
    add_to_mem_block(A_INCLUDES, name, strlen(name) + 1);
  }
  return store_prog_string(name) + 1;
}

char* allocate_in_mem_block(int n, int size) {
  mem_block_t* mbp = &mem_block[n];
  char* ret;

  if (mbp->current_size + size > mbp->max_size) {
    do {
      mbp->max_size <<= 1;
    } while (mbp->current_size + size > mbp->max_size);

    mbp->block = reinterpret_cast<char*>(
        DREALLOC(mbp->block, mbp->max_size, TAG_COMPILER, "insert_in_mem_block"));
  }
  ret = mbp->block + mbp->current_size;
  mbp->current_size += size;
  return ret;
}
