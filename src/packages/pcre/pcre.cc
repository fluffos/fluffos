/* FILES
 *    - pcre.c
 *    - pcre_spec.c
 *    - pcre.h
 *
 * PCRE (Perl Compatible Regular Expressions)
 *    Efuns using PCRE2 (https://github.com/PCRE2Project/pcre2).
 *    UTF-8, Unicode properties, and JIT (when available) are on by default.
 *
 *    For regular expressions syntactics:
 *       http://perldoc.perl.org/perlre.html
 *
 *
 * AUTHOR
 *    Volothamp @ Final Realms
 *         nfa106 [a] gmail.com
 *
 * HISTORY
 *    June-July 2009:
 *       Package created
 *
 *
 * DESCRIPTION
 *
 *   This package introduces the following efuns:
 *
 *      string pcre_version();
 *         - returns the version of the compiled PCRE2 library used
 *
 *      mapping pcre_config();
 *         - PCRE2 library capabilities (JIT, Unicode, default limits)
 *
 *      mapping pcre_info(string, void | int | mapping);
 *         - compiled pattern metadata (captures, names, JIT, limits)
 *
 *      string pcre_convert(string, mapping);
 *         - glob / POSIX pattern → PCRE2 pattern
 *
 *      mixed pcre_match(string | string *, string, void | int);
 *         - analog with regexp(string | string *, string, void | int);
 *           for backwards compatibility reasons but utilizing the PCRE
 *           library.
 *
 *      mixed *pcre_assoc(string, string *, mixed *, mixed | void);
 *         - analog with reg_assoc(string, string *, mixed *, mixed | void);
 *           for backwards compatibility reasons but utilizing the PCRE
 *           library.
 *
 *      string *pcre_extract(string subject, string pattern);
 *         - returns an array of captured groups specified in pattern
 *
 *      string pcre_replace(string subject, string pattern, string
 **replacement);
 *         - returns a string where all captured groups have been replaced by
 *the
 *           elements of the replacement array. Number of subgroups and the size
 *of
 *           the replacement array must match.
 *
 *      string pcre_replace_callback(string subject, string pattern, function |
 *string fun, object ob);
 *         - returns a string where all captured groups have been replaced by
 *the
 *           return value of function pointe fun or function fun in object ob.
 */
// TODO
// store reg->error & reg->erroffset before error(..)

#include <cstring>
#include <thirdparty/scope_guard/scope_guard.hpp>
#include "base/package_api.h"

#include "pcre.h"
#include "include/pcre_flags.h"
#include "vm/internal/base/mapping.h"

struct pcre_options {
  uint32_t compile_flags;
  uint32_t extra_options;
  uint32_t newline;
  uint32_t bsr;
  uint32_t exec_flags;
  uint32_t substitute_flags;
  uint32_t match_limit;
  uint32_t depth_limit;
  uint32_t heap_limit;
  PCRE2_SIZE offset_limit;
  PCRE2_SIZE start_offset;
  int has_match_limit;
  int has_depth_limit;
  int has_heap_limit;
  int has_offset_limit;
  int has_newline;
  int has_bsr;
  int no_jit;
};

// Prototype declarations
static void pcre_free_memory(pcre_t* p);
static pcre2_code* pcre_local_compile(pcre_t* p);
static int pcre_ensure_compiled(pcre_t* p);
static int pcre_local_exec(pcre_t* p);
static int pcre_exec_at(pcre_t* p, size_t offset, int extra_flags);
static int pcre_magic(pcre_t* p);
static char* pcre_substitute_template(pcre_t* run, const char* repl, size_t repl_len);
static int pcre_query_match(pcre_t* p);
static pcre_options default_pcre_options();
static void apply_int_flags(pcre_options* o, LPC_INT flags);
static void apply_mapping_flags(pcre_options* o, mapping_t* map);
static void apply_options_svalue(pcre_options* o, svalue_t* sv);
static void assign_run_options(pcre_t* run, const pcre_options& o);
static pcre2_match_context* pcre_run_context(pcre_t* p);
static void pcre_throw_if_limit_error(int rc);
static int pcre_match_single(svalue_t* str, const char* pattern, const pcre_options& o);
static array_t* pcre_match(array_t* v, const char* pattern, int flag, const pcre_options& o);
static array_t* pcre_assoc(svalue_t* str, array_t* pat, array_t* tok, svalue_t* def,
                           const pcre_options& o);
static char* pcre_get_replace(pcre_t* run, array_t* replacements);
static array_t* pcre_get_substrings(pcre_t* run, bool include_names);
// Caching functions
static int pcre_cache_pattern(struct pcre_cache_t* table, pcre2_code* cpat, const pcre_t* p);
static pcre2_code* pcre_get_cached_pattern(struct pcre_cache_t* table, const pcre_t* p);
static mapping_t* pcre_get_cache();
int pcrecachesize = 0;
// Globals
struct pcre_cache_t pcre_cache = {{nullptr}};

// efuns
void f_pcre_version() {
  char version[96];
  int jit = 0;
  if (pcre2_config(PCRE2_CONFIG_VERSION, version) < 0) {
    push_constant_string("unknown");
    return;
  }
  pcre2_config(PCRE2_CONFIG_JIT, &jit);
  if (jit) {
    size_t n = strlen(version);
    if (n + 5 < sizeof(version)) {
      memcpy(version + n, " JIT", 5);
    }
  }
  copy_and_push_string(version);
}

void f_pcre_config() {
  mapping_t* m = allocate_mapping(16);
  char buf[128];
  uint32_t u = 0;

  if (pcre2_config(PCRE2_CONFIG_VERSION, buf) >= 0) {
    add_mapping_string(m, "version", buf);
  }
  if (pcre2_config(PCRE2_CONFIG_JIT, &u) >= 0) {
    add_mapping_pair(m, "jit", u);
#ifdef PCRE2_CONFIG_JITTARGET
    if (u && pcre2_config(PCRE2_CONFIG_JITTARGET, buf) >= 0) {
      add_mapping_string(m, "jit_target", buf);
    }
#endif
  }
  if (pcre2_config(PCRE2_CONFIG_UNICODE, &u) >= 0) {
    add_mapping_pair(m, "unicode", u);
  }
  if (pcre2_config(PCRE2_CONFIG_UNICODE_VERSION, buf) >= 0) {
    add_mapping_string(m, "unicode_version", buf);
  }
  if (pcre2_config(PCRE2_CONFIG_NEWLINE, &u) >= 0) {
    add_mapping_pair(m, "newline", u);
  }
  if (pcre2_config(PCRE2_CONFIG_BSR, &u) >= 0) {
    add_mapping_pair(m, "bsr", u);
  }
  if (pcre2_config(PCRE2_CONFIG_LINKSIZE, &u) >= 0) {
    add_mapping_pair(m, "link_size", u);
  }
#ifdef PCRE2_CONFIG_HEAPLIMIT
  if (pcre2_config(PCRE2_CONFIG_HEAPLIMIT, &u) >= 0) {
    add_mapping_pair(m, "heap_limit", u);
  }
#endif
  if (pcre2_config(PCRE2_CONFIG_MATCHLIMIT, &u) >= 0) {
    add_mapping_pair(m, "match_limit", u);
  }
  if (pcre2_config(PCRE2_CONFIG_DEPTHLIMIT, &u) >= 0) {
    add_mapping_pair(m, "depth_limit", u);
  }
#ifdef PCRE2_CONFIG_NEVER_BACKSLASH_C
  if (pcre2_config(PCRE2_CONFIG_NEVER_BACKSLASH_C, &u) >= 0) {
    add_mapping_pair(m, "never_backslash_c", u);
  }
#endif
  push_refed_mapping(m);
}

void f_pcre_info() {
  pcre_options opts = default_pcre_options();
  if (st_num_arg >= 2) {
    apply_options_svalue(&opts, sp);
    pop_stack();
    st_num_arg--;
  }
  if (sp->type != T_STRING) {
    error("Bad argument 1 to pcre_info()\n");
  }

  pcre_t* run = (pcre_t*)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "f_pcre_info : run");
  run->pattern = sp->u.string;
  assign_run_options(run, opts);
  DEFER { pcre_free_memory(run); };

  if (pcre_ensure_compiled(run) < 0) {
    error("PCRE compilation failed at offset %d: %s\n", run->erroffset, run->error);
  }

  mapping_t* m = allocate_mapping(20);
  uint32_t u = 0;
  PCRE2_SIZE sz = 0;

  if (pcre2_pattern_info(run->re, PCRE2_INFO_CAPTURECOUNT, &u) == 0) {
    add_mapping_pair(m, "captures", u);
  }
  if (pcre2_pattern_info(run->re, PCRE2_INFO_BACKREFMAX, &u) == 0) {
    add_mapping_pair(m, "backref_max", u);
  }
  if (pcre2_pattern_info(run->re, PCRE2_INFO_MINLENGTH, &u) == 0) {
    add_mapping_pair(m, "minlength", u);
  }
  if (pcre2_pattern_info(run->re, PCRE2_INFO_MAXLOOKBEHIND, &u) == 0) {
    add_mapping_pair(m, "max_lookbehind", u);
  }
  if (pcre2_pattern_info(run->re, PCRE2_INFO_MATCHEMPTY, &u) == 0) {
    add_mapping_pair(m, "match_empty", u);
  }
  if (pcre2_pattern_info(run->re, PCRE2_INFO_HASCRORLF, &u) == 0) {
    add_mapping_pair(m, "has_cr_or_lf", u);
  }
  if (pcre2_pattern_info(run->re, PCRE2_INFO_HASBACKSLASHC, &u) == 0) {
    add_mapping_pair(m, "has_backslash_c", u);
  }
  if (pcre2_pattern_info(run->re, PCRE2_INFO_NEWLINE, &u) == 0) {
    add_mapping_pair(m, "newline", u);
  }
  if (pcre2_pattern_info(run->re, PCRE2_INFO_BSR, &u) == 0) {
    add_mapping_pair(m, "bsr", u);
  }
  if (pcre2_pattern_info(run->re, PCRE2_INFO_SIZE, &sz) == 0) {
    add_mapping_pair(m, "size", static_cast<long>(sz));
  }
#ifdef PCRE2_INFO_JITSIZE
  if (pcre2_pattern_info(run->re, PCRE2_INFO_JITSIZE, &sz) == 0) {
    add_mapping_pair(m, "jit_size", static_cast<long>(sz));
    add_mapping_pair(m, "jit", sz > 0 ? 1 : 0);
  }
#endif

  uint32_t namecount = 0, name_entry_size = 0;
  PCRE2_SPTR name_table = nullptr;
  pcre2_pattern_info(run->re, PCRE2_INFO_NAMECOUNT, &namecount);
  pcre2_pattern_info(run->re, PCRE2_INFO_NAMEENTRYSIZE, &name_entry_size);
  pcre2_pattern_info(run->re, PCRE2_INFO_NAMETABLE, &name_table);
  mapping_t* names = allocate_mapping(namecount);
  for (uint32_t i = 0; i < namecount && name_table != nullptr; i++) {
    auto* entry = name_table + i * name_entry_size;
    int const group = (entry[0] << 8) | entry[1];
    add_mapping_pair(names, reinterpret_cast<const char*>(entry + 2), group);
  }
  {
    svalue_t key;
    key.type = T_STRING;
    key.subtype = STRING_SHARED;
    key.u.string = make_shared_string("names");
    svalue_t* slot = find_for_insert(m, &key, 1);
    free_string(key.u.string);
    if (slot != nullptr) {
      slot->type = T_MAPPING;
      slot->subtype = 0;
      slot->u.map = names;
    } else {
      free_mapping(names);
    }
  }

  pop_stack();
  push_refed_mapping(m);
}

void f_pcre_convert() {
  uint32_t cflags = PCRE2_CONVERT_UTF;
  int have_kind = 0;

  if (st_num_arg < 2 || sp->type != T_MAPPING) {
    error("pcre_convert() requires a mapping of convert options (glob, posix_basic, or posix_extended).\n");
  }

  mapping_t* map = sp->u.map;
  auto on = [&](const char* key) {
    svalue_t* v = find_string_in_mapping(map, key);
    return v && v->type == T_NUMBER && v->u.number != 0;
  };
  if (on("glob")) {
    cflags |= PCRE2_CONVERT_GLOB;
    have_kind = 1;
  }
  if (on("posix_basic")) {
    cflags |= PCRE2_CONVERT_POSIX_BASIC;
    have_kind = 1;
  }
  if (on("posix_extended")) {
    cflags |= PCRE2_CONVERT_POSIX_EXTENDED;
    have_kind = 1;
  }
  if (on("glob_no_starstar")) {
    cflags |= PCRE2_CONVERT_GLOB_NO_STARSTAR;
  }
  if (on("glob_no_wild_separator")) {
    cflags |= PCRE2_CONVERT_GLOB_NO_WILD_SEPARATOR;
  }
  if (on("no_utf_check")) {
    cflags |= PCRE2_CONVERT_NO_UTF_CHECK;
  }
  if (!have_kind) {
    error("pcre_convert() mapping must set glob, posix_basic, or posix_extended.\n");
  }

  const char* pat = (sp - 1)->u.string;
  PCRE2_UCHAR* out = nullptr;
  PCRE2_SIZE outlen = 0;
  int rc = pcre2_pattern_convert((PCRE2_SPTR)pat, PCRE2_ZERO_TERMINATED, cflags, &out, &outlen,
                                 nullptr);
  if (rc < 0) {
    char err[256];
    pcre2_get_error_message(rc, reinterpret_cast<PCRE2_UCHAR*>(err), sizeof(err));
    error("PCRE convert failed: %s\n", err);
  }
  char* ret = new_string(static_cast<int>(outlen), "pcre_convert");
  memcpy(ret, out, outlen);
  ret[outlen] = '\0';
  pcre2_converted_pattern_free(out);
  pop_2_elems();
  push_malloced_string(ret);
}

void f_pcre_match() {
  array_t* v;
  int flag = 0;
  pcre_options opts = default_pcre_options();
  // The subject (1st arg) sits at sp - st_num_arg + 1 regardless of how
  // many optional trailing args (flag, pcre_flags) are present; reading
  // (sp - 1) here only happened to be correct for the 2-arg call form --
  // with a 3rd (or 4th) argument on the stack, sp - 1 is the pattern (or
  // flag) instead, always misidentifying array-mode calls as string-mode.
  bool is_string = ((sp - st_num_arg + 1)->type == T_STRING);

  // optional 4th arg: pcre_flags (int or mapping)
  if (st_num_arg > 3) {
    apply_options_svalue(&opts, sp);
    pop_stack();
    st_num_arg--;
  }

  // optional 3rd arg:
  if (st_num_arg == 3) {
    if (is_string) {
      apply_options_svalue(&opts, sp);
      pop_stack();
    } else if (sp->type == T_MAPPING) {
      apply_options_svalue(&opts, sp);
      pop_stack();
    } else if (sp->type == T_NUMBER) {
      flag = (sp--)->u.number;
    } else {
      error("Bad argument 3 to pcre_match()\n");
    }
    st_num_arg--;
  }

  if (is_string) {
    flag = pcre_match_single((sp - 1), sp->u.string, opts);

    free_string_svalue(sp--);
    free_string_svalue(sp);
    put_number(flag);
  } else {
    v = pcre_match((sp - 1)->u.arr, sp->u.string, flag, opts);

    free_string_svalue(sp--);
    free_array(sp->u.arr);
    sp->u.arr = v;
  }
}

void f_pcre_assoc() {
  svalue_t* arg;
  array_t* vec;
  pcre_options opts = default_pcre_options();

  if (st_num_arg == 5) {
    apply_options_svalue(&opts, sp);
    pop_stack();
    st_num_arg--;
  }

  arg = sp - st_num_arg + 1;

  if ((arg + 2)->type != T_ARRAY) {
    error("Bad argument 3 to pcre_assoc()\n");
  }

  vec = pcre_assoc(arg, (arg + 1)->u.arr, (arg + 2)->u.arr, st_num_arg > 3 ? (arg + 3) : &const0,
                   opts);

  if (st_num_arg == 4) {
    pop_3_elems();
  } else {
    pop_2_elems();
  }

  free_string_svalue(sp);

  sp->type = T_ARRAY;
  sp->u.arr = vec;
}

void f_pcre_extract() {
  pcre_t* run;
  array_t* ret;
  int include_names = 0;
  pcre_options opts = default_pcre_options();

  if (st_num_arg >= 4) {
    apply_options_svalue(&opts, sp);
    pop_stack();
    st_num_arg--;
  }

  svalue_t* arg = sp - st_num_arg + 1;

  if (st_num_arg == 3) {
    if ((arg + 2)->type == T_MAPPING) {
      apply_options_svalue(&opts, arg + 2);
    } else if ((arg + 2)->type == T_NUMBER) {
      include_names = (arg + 2)->u.number != 0;
    } else {
      error("Bad argument 3 to pcre_extract()\n");
    }
  } else if (st_num_arg != 2) {
    error("pcre_extract() requires 2 or 3 arguments\n");
  }

  run = (pcre_t*)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "f_pcre_extract : run");
  run->pattern = (arg + 1)->u.string;
  run->subject = arg->u.string;
  run->s_length = SVALUE_STRLEN(arg);
  run->ovector = nullptr;
  run->ovecsize = 0;
  assign_run_options(run, opts);
  DEFER { pcre_free_memory(run); };

  if (pcre_magic(run) < 0) {
    error("PCRE compilation failed at offset %d: %s\n", run->erroffset, run->error);
  }

  if (run->rc < 0) { /* No match. could do handling of matching errors if wanted */
    pop_n_elems(st_num_arg);
    push_refed_array(&the_null_array);
    return;
  }
  if (run->rc > 0 && run->rc * 2 > run->ovecsize) {
    error("Too many substrings.\n");
  }

  ret = pcre_get_substrings(run, include_names);
  pop_n_elems(st_num_arg);

  push_refed_array(ret);
}

void f_pcre_replace() {
  pcre_t* run;
  array_t* replacements;

  char* ret;
  pcre_options opts = default_pcre_options();

  if (st_num_arg >= 4) {
    apply_options_svalue(&opts, sp);
    pop_stack();
    st_num_arg--;
  }

  if (sp->type != T_ARRAY && sp->type != T_STRING) {
    error("Bad argument 3 to pcre_replace()\n");
  }

  run = (pcre_t*)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "f_pcre_replace: run");

  run->ovector = nullptr;
  run->ovecsize = 0;
  run->pattern = (sp - 1)->u.string;
  run->subject = (sp - 2)->u.string;
  run->s_length = SVALUE_STRLEN(sp - 2);
  assign_run_options(run, opts);
  DEFER { pcre_free_memory(run); };

  if (sp->type == T_STRING) {
    if (pcre_ensure_compiled(run) < 0) {
      error("PCRE compilation failed at offset %d: %s\n", run->erroffset, run->error);
    }
    ret = pcre_substitute_template(run, sp->u.string, SVALUE_STRLEN(sp));
    pop_3_elems();
    push_malloced_string(ret);
    return;
  }

  replacements = sp->u.arr;

  if (pcre_magic(run) < 0) {
    error("PCRE compilation failed at offset %d: %s\n", run->erroffset, run->error);
  }

  if (run->rc < 0) { /* No match. could do handling of matching errors if wanted */
    pop_2_elems();
    return;
  }

  if (run->rc > 0 && run->rc * 2 > run->ovecsize) {
    error("Too many substrings.\n");
  }
  if ((run->rc - 1) != replacements->size) {
    int const tmp = run->rc - 1;
    error(
        "Number of captured substrings and replacements do not match, "
        "%d vs %d.\n",
        tmp, replacements->size);
  }

  if (run->rc == 1) {
    /* No captured substrings, return subject */
    pop_2_elems();
    return;
    // push_malloced_string(run->subject);
  }

  for (int i = 0; i < replacements->size; i++) {
    if (replacements->item[i].type != T_STRING) {
      error("Bad argument 3 to pcre_replace(): replacement array must contain only strings.\n");
    }
  }

  ret = pcre_get_replace(run, replacements);

  pop_3_elems();
  push_malloced_string(ret);
}

// string pcre_replace_callback(string, string, function)
void f_pcre_replace_callback() {
  int num_arg = st_num_arg, i;
  char* ret;
  pcre_t* run;
  svalue_t* arg;
  array_t *arr, *r;
  function_to_call_t ftc;
  pcre_options opts = default_pcre_options();

  if (num_arg >= 4 && (sp->type == T_NUMBER || sp->type == T_MAPPING)) {
    apply_options_svalue(&opts, sp);
    pop_stack();
    st_num_arg--;
    num_arg--;
  }

  arg = sp - num_arg + 1;

  run = (pcre_t*)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "f_pcre_replace: run");
  run->ovector = nullptr;
  run->ovecsize = 0;
  run->subject = arg->u.string;
  run->pattern = (arg + 1)->u.string;

  run->s_length = SVALUE_STRLEN(arg);
  assign_run_options(run, opts);
  DEFER { pcre_free_memory(run); };

  if (pcre_magic(run) < 0) {
    error("PCRE compilation failed at offset %d: %s\n", run->erroffset, run->error);
  }

  if (run->rc < 0) { /* No match. could do handling of matching errors if wanted */
    pop_n_elems(num_arg - 1);
    return;
  }

  if (run->rc > 0 && run->rc * 2 > run->ovecsize) {
    error("Too many substrings.\n");
  }

  arr = pcre_get_substrings(run, false);

  if (arg[2].type == T_FUNCTION || arg[2].type == T_STRING) {
    process_efun_callback(2, &ftc, F_PCRE_REPLACE_CALLBACK);
  } else {  // 0
    free_array(arr);
    error("Illegal third argument (0) to pcre_replace_callback");
  }

  r = allocate_array(run->rc - 1);  // can't use the empty variant in case we error below

  push_refed_array(r);
  push_refed_array(arr);
  error_context_t econ;

  save_context(&econ);
  try {
    for (i = 0; i < run->rc - 1; i++) {
      svalue_t* v;
      push_svalue(arr->item + i);
      push_number(i);
      v = call_efun_callback(&ftc, 2);

      /* Mimic behaviour of map(string, function) when function pointer returns
       null,
       ie return the input.  */
      if (v && v->type == T_STRING && v->u.string != nullptr) {
        assign_svalue_no_free(&r->item[i], v);
      } else {
        assign_svalue_no_free(&r->item[i], &arr->item[i]);
      }
    }
  } catch (const char*) {
    restore_context(&econ);
    /* condition was restored to where it was when we came in */
    pop_context(&econ);
    error("error in callback!\n");
  }
  pop_context(&econ);
  ret = pcre_get_replace(run, r);

  pop_n_elems(num_arg + 2);  // refed arrays
  push_malloced_string(ret);
}

void f_pcre_cache() {
  mapping_t* m = nullptr;
  m = pcre_get_cache();
  if (!m) {
    push_number(0);
  } else {
    push_refed_mapping(m);
  }
}

// Internal functions utilized by the efuns
static pcre_options default_pcre_options() {
  pcre_options o{};
  o.compile_flags = PCRE2_UTF | PCRE2_UCP;
#ifdef PCRE2_MATCH_INVALID_UTF
  o.compile_flags |= PCRE2_MATCH_INVALID_UTF;
#endif
  o.substitute_flags = PCRE2_SUBSTITUTE_GLOBAL | PCRE2_SUBSTITUTE_EXTENDED |
                       PCRE2_SUBSTITUTE_OVERFLOW_LENGTH | PCRE2_SUBSTITUTE_UNSET_EMPTY |
                       PCRE2_SUBSTITUTE_UNKNOWN_UNSET;
  return o;
}

static void apply_int_flags(pcre_options* o, LPC_INT flags) {
  if (flags & PCRE_NO_UCP) o->compile_flags &= ~PCRE2_UCP;
  if (flags & PCRE_I) o->compile_flags |= PCRE2_CASELESS;
  if (flags & PCRE_M) o->compile_flags |= PCRE2_MULTILINE;
  if (flags & PCRE_S) o->compile_flags |= PCRE2_DOTALL;
  if (flags & PCRE_U) o->compile_flags |= PCRE2_UNGREEDY;
  if (flags & PCRE_X) o->compile_flags |= PCRE2_EXTENDED;
  if (flags & PCRE_ENDANCHOR) o->compile_flags |= PCRE2_ENDANCHORED;
  if (flags & PCRE_NO_AUTO_CAPTURE) o->compile_flags |= PCRE2_NO_AUTO_CAPTURE;
  if (flags & PCRE_FIRSTLINE) o->compile_flags |= PCRE2_FIRSTLINE;
  if (flags & PCRE_DUPNAMES) o->compile_flags |= PCRE2_DUPNAMES;
  if (flags & PCRE_A) o->exec_flags |= PCRE2_ANCHORED;
  if (flags & PCRE_NOTEMPTY) o->exec_flags |= PCRE2_NOTEMPTY;
  if (flags & PCRE_NOTBOL) o->exec_flags |= PCRE2_NOTBOL;
  if (flags & PCRE_NOTEOL) o->exec_flags |= PCRE2_NOTEOL;
  if (flags & PCRE_PARTIAL) o->exec_flags |= PCRE2_PARTIAL_SOFT;
}

static int sval_truthy(const svalue_t* v, const char* key) {
  if (v->type == T_NUMBER) {
    return v->u.number != 0;
  }
  error("PCRE option \"%s\" must be a number.\n", key);
  return 0;
}

static LPC_INT sval_int(const svalue_t* v, const char* key) {
  if (v->type != T_NUMBER) {
    error("PCRE option \"%s\" must be an int.\n", key);
  }
  return v->u.number;
}

static void set_compile_bit(pcre_options* o, uint32_t bit, int on) {
  if (on) {
    o->compile_flags |= bit;
  } else {
    o->compile_flags &= ~bit;
  }
}

static void set_exec_bit(pcre_options* o, uint32_t bit, int on) {
  if (on) {
    o->exec_flags |= bit;
  } else {
    o->exec_flags &= ~bit;
  }
}

static void set_sub_bit(pcre_options* o, uint32_t bit, int on) {
  if (on) {
    o->substitute_flags |= bit;
  } else {
    o->substitute_flags &= ~bit;
  }
}

static void set_extra_bit(pcre_options* o, uint32_t bit, int on) {
  if (on) {
    o->extra_options |= bit;
  } else {
    o->extra_options &= ~bit;
  }
}

static int apply_one_option(mapping_t* /*m*/, mapping_node_t* n, void* extra) {
  auto* o = static_cast<pcre_options*>(extra);
  const svalue_t* key = n->values;
  const svalue_t* val = n->values + 1;
  if (key->type != T_STRING) {
    error("PCRE option keys must be strings.\n");
  }
  const char* k = key->u.string;
  auto on = [&](const char* name) { return sval_truthy(val, name); };

  if (strcmp(k, "flags") == 0) {
    apply_int_flags(o, sval_int(val, k));
  } else if (strcmp(k, "caseless") == 0 || strcmp(k, "i") == 0) {
    set_compile_bit(o, PCRE2_CASELESS, on(k));
  } else if (strcmp(k, "multiline") == 0 || strcmp(k, "m") == 0) {
    set_compile_bit(o, PCRE2_MULTILINE, on(k));
  } else if (strcmp(k, "dotall") == 0 || strcmp(k, "s") == 0) {
    set_compile_bit(o, PCRE2_DOTALL, on(k));
  } else if (strcmp(k, "ungreedy") == 0 || strcmp(k, "u") == 0) {
    set_compile_bit(o, PCRE2_UNGREEDY, on(k));
  } else if (strcmp(k, "extended") == 0 || strcmp(k, "x") == 0) {
    set_compile_bit(o, PCRE2_EXTENDED, on(k));
#ifdef PCRE2_EXTENDED_MORE
  } else if (strcmp(k, "extended_more") == 0) {
    set_compile_bit(o, PCRE2_EXTENDED_MORE, on(k));
#endif
  } else if (strcmp(k, "endanchored") == 0 || strcmp(k, "endanchor") == 0) {
    set_compile_bit(o, PCRE2_ENDANCHORED, on(k));
  } else if (strcmp(k, "no_auto_capture") == 0) {
    set_compile_bit(o, PCRE2_NO_AUTO_CAPTURE, on(k));
  } else if (strcmp(k, "firstline") == 0) {
    set_compile_bit(o, PCRE2_FIRSTLINE, on(k));
  } else if (strcmp(k, "dupnames") == 0) {
    set_compile_bit(o, PCRE2_DUPNAMES, on(k));
#ifdef PCRE2_LITERAL
  } else if (strcmp(k, "literal") == 0) {
    set_compile_bit(o, PCRE2_LITERAL, on(k));
#endif
  } else if (strcmp(k, "dollar_endonly") == 0) {
    set_compile_bit(o, PCRE2_DOLLAR_ENDONLY, on(k));
  } else if (strcmp(k, "alt_bsux") == 0) {
    set_compile_bit(o, PCRE2_ALT_BSUX, on(k));
  } else if (strcmp(k, "alt_circumflex") == 0) {
    set_compile_bit(o, PCRE2_ALT_CIRCUMFLEX, on(k));
#ifdef PCRE2_ALT_VERBNAMES
  } else if (strcmp(k, "alt_verbnames") == 0) {
    set_compile_bit(o, PCRE2_ALT_VERBNAMES, on(k));
#endif
  } else if (strcmp(k, "allow_empty_class") == 0) {
    set_compile_bit(o, PCRE2_ALLOW_EMPTY_CLASS, on(k));
  } else if (strcmp(k, "match_unset_backref") == 0) {
    set_compile_bit(o, PCRE2_MATCH_UNSET_BACKREF, on(k));
  } else if (strcmp(k, "no_auto_possess") == 0) {
    set_compile_bit(o, PCRE2_NO_AUTO_POSSESS, on(k));
#ifdef PCRE2_NO_DOTSTAR_ANCHOR
  } else if (strcmp(k, "no_dotstar_anchor") == 0) {
    set_compile_bit(o, PCRE2_NO_DOTSTAR_ANCHOR, on(k));
#endif
  } else if (strcmp(k, "no_start_optimize") == 0) {
    set_compile_bit(o, PCRE2_NO_START_OPTIMIZE, on(k));
#ifdef PCRE2_NO_UTF_CHECK
  } else if (strcmp(k, "no_utf_check") == 0) {
    set_compile_bit(o, PCRE2_NO_UTF_CHECK, on(k));
#endif
#ifdef PCRE2_USE_OFFSET_LIMIT
  } else if (strcmp(k, "use_offset_limit") == 0) {
    set_compile_bit(o, PCRE2_USE_OFFSET_LIMIT, on(k));
#endif
  } else if (strcmp(k, "no_ucp") == 0) {
    if (on(k)) {
      o->compile_flags &= ~PCRE2_UCP;
    } else {
      o->compile_flags |= PCRE2_UCP;
    }
  } else if (strcmp(k, "ucp") == 0) {
    set_compile_bit(o, PCRE2_UCP, on(k));
  } else if (strcmp(k, "anchored") == 0 || strcmp(k, "a") == 0) {
    set_exec_bit(o, PCRE2_ANCHORED, on(k));
  } else if (strcmp(k, "notempty") == 0) {
    set_exec_bit(o, PCRE2_NOTEMPTY, on(k));
  } else if (strcmp(k, "notbol") == 0) {
    set_exec_bit(o, PCRE2_NOTBOL, on(k));
  } else if (strcmp(k, "noteol") == 0) {
    set_exec_bit(o, PCRE2_NOTEOL, on(k));
  } else if (strcmp(k, "partial") == 0 || strcmp(k, "partial_soft") == 0) {
    set_exec_bit(o, PCRE2_PARTIAL_SOFT, on(k));
  } else if (strcmp(k, "partial_hard") == 0) {
    set_exec_bit(o, PCRE2_PARTIAL_HARD, on(k));
#ifdef PCRE2_NOTEMPTY_ATSTART
  } else if (strcmp(k, "notempty_atstart") == 0) {
    set_exec_bit(o, PCRE2_NOTEMPTY_ATSTART, on(k));
#endif
  } else if (strcmp(k, "replace_global") == 0) {
    set_sub_bit(o, PCRE2_SUBSTITUTE_GLOBAL, on(k));
  } else if (strcmp(k, "replace_extended") == 0) {
    set_sub_bit(o, PCRE2_SUBSTITUTE_EXTENDED, on(k));
#ifdef PCRE2_SUBSTITUTE_LITERAL
    if (on(k)) {
      o->substitute_flags &= ~PCRE2_SUBSTITUTE_LITERAL;
    }
#endif
#ifdef PCRE2_SUBSTITUTE_LITERAL
  } else if (strcmp(k, "replace_literal") == 0) {
    set_sub_bit(o, PCRE2_SUBSTITUTE_LITERAL, on(k));
    if (on(k)) {
      o->substitute_flags &= ~PCRE2_SUBSTITUTE_EXTENDED;
    }
#endif
#ifdef PCRE2_SUBSTITUTE_REPLACEMENT_ONLY
  } else if (strcmp(k, "replace_replacement_only") == 0) {
    set_sub_bit(o, PCRE2_SUBSTITUTE_REPLACEMENT_ONLY, on(k));
#endif
#ifdef PCRE2_SUBSTITUTE_MATCHED
  } else if (strcmp(k, "replace_matched") == 0) {
    set_sub_bit(o, PCRE2_SUBSTITUTE_MATCHED, on(k));
#endif
  } else if (strcmp(k, "replace_unknown_unset") == 0) {
    set_sub_bit(o, PCRE2_SUBSTITUTE_UNKNOWN_UNSET, on(k));
  } else if (strcmp(k, "replace_unset_empty") == 0) {
    set_sub_bit(o, PCRE2_SUBSTITUTE_UNSET_EMPTY, on(k));
  } else if (strcmp(k, "match_limit") == 0) {
    LPC_INT n = sval_int(val, k);
    if (n < 0) error("PCRE option \"match_limit\" must be >= 0.\n");
    o->match_limit = static_cast<uint32_t>(n);
    o->has_match_limit = 1;
  } else if (strcmp(k, "depth_limit") == 0) {
    LPC_INT n = sval_int(val, k);
    if (n < 0) error("PCRE option \"depth_limit\" must be >= 0.\n");
    o->depth_limit = static_cast<uint32_t>(n);
    o->has_depth_limit = 1;
  } else if (strcmp(k, "heap_limit") == 0) {
    LPC_INT n = sval_int(val, k);
    if (n < 0) error("PCRE option \"heap_limit\" must be >= 0.\n");
    o->heap_limit = static_cast<uint32_t>(n);
    o->has_heap_limit = 1;
  } else if (strcmp(k, "offset_limit") == 0) {
    LPC_INT n = sval_int(val, k);
    if (n < 0) error("PCRE option \"offset_limit\" must be >= 0.\n");
    o->offset_limit = static_cast<PCRE2_SIZE>(n);
    o->has_offset_limit = 1;
  } else if (strcmp(k, "offset") == 0) {
    LPC_INT n = sval_int(val, k);
    if (n < 0) error("PCRE option \"offset\" must be >= 0.\n");
    o->start_offset = static_cast<PCRE2_SIZE>(n);
  } else if (strcmp(k, "no_jit") == 0) {
    o->no_jit = on(k);
  } else if (strcmp(k, "newline") == 0) {
    if (val->type == T_NUMBER) {
      LPC_INT n = val->u.number;
      if (n < PCRE2_NEWLINE_CR || n > PCRE2_NEWLINE_NUL) {
        error("PCRE option \"newline\" must be cr/lf/crlf/any/anycrlf/nul.\n");
      }
      o->newline = static_cast<uint32_t>(n);
    } else if (val->type == T_STRING) {
      const char* s = val->u.string;
      if (strcmp(s, "cr") == 0) {
        o->newline = PCRE2_NEWLINE_CR;
      } else if (strcmp(s, "lf") == 0) {
        o->newline = PCRE2_NEWLINE_LF;
      } else if (strcmp(s, "crlf") == 0) {
        o->newline = PCRE2_NEWLINE_CRLF;
      } else if (strcmp(s, "any") == 0) {
        o->newline = PCRE2_NEWLINE_ANY;
      } else if (strcmp(s, "anycrlf") == 0) {
        o->newline = PCRE2_NEWLINE_ANYCRLF;
      } else if (strcmp(s, "nul") == 0) {
        o->newline = PCRE2_NEWLINE_NUL;
      } else {
        error("PCRE option \"newline\" must be cr/lf/crlf/any/anycrlf/nul.\n");
      }
    } else {
      error("PCRE option \"newline\" must be a string or int.\n");
    }
    o->has_newline = 1;
  } else if (strcmp(k, "bsr") == 0) {
    if (val->type == T_NUMBER) {
      LPC_INT n = val->u.number;
      if (n != PCRE2_BSR_UNICODE && n != PCRE2_BSR_ANYCRLF) {
        error("PCRE option \"bsr\" must be unicode or anycrlf.\n");
      }
      o->bsr = static_cast<uint32_t>(n);
    } else if (val->type == T_STRING) {
      const char* s = val->u.string;
      if (strcmp(s, "unicode") == 0) {
        o->bsr = PCRE2_BSR_UNICODE;
      } else if (strcmp(s, "anycrlf") == 0) {
        o->bsr = PCRE2_BSR_ANYCRLF;
      } else {
        error("PCRE option \"bsr\" must be unicode or anycrlf.\n");
      }
    } else {
      error("PCRE option \"bsr\" must be a string or int.\n");
    }
    o->has_bsr = 1;
#ifdef PCRE2_EXTRA_ALLOW_SURROGATE_ESCAPES
  } else if (strcmp(k, "extra_allow_surrogate_escapes") == 0) {
    set_extra_bit(o, PCRE2_EXTRA_ALLOW_SURROGATE_ESCAPES, on(k));
#endif
#ifdef PCRE2_EXTRA_BAD_ESCAPE_IS_LITERAL
  } else if (strcmp(k, "extra_bad_escape_is_literal") == 0) {
    set_extra_bit(o, PCRE2_EXTRA_BAD_ESCAPE_IS_LITERAL, on(k));
#endif
#ifdef PCRE2_EXTRA_MATCH_WORD
  } else if (strcmp(k, "extra_match_word") == 0) {
    set_extra_bit(o, PCRE2_EXTRA_MATCH_WORD, on(k));
#endif
#ifdef PCRE2_EXTRA_MATCH_LINE
  } else if (strcmp(k, "extra_match_line") == 0) {
    set_extra_bit(o, PCRE2_EXTRA_MATCH_LINE, on(k));
#endif
#ifdef PCRE2_EXTRA_ESCAPED_CR_IS_LF
  } else if (strcmp(k, "extra_escaped_cr_is_lf") == 0) {
    set_extra_bit(o, PCRE2_EXTRA_ESCAPED_CR_IS_LF, on(k));
#endif
#ifdef PCRE2_EXTRA_ALT_BSUX
  } else if (strcmp(k, "extra_alt_bsux") == 0) {
    set_extra_bit(o, PCRE2_EXTRA_ALT_BSUX, on(k));
#endif
#ifdef PCRE2_EXTRA_ALLOW_LOOKAROUND_BSK
  } else if (strcmp(k, "extra_allow_lookaround_bsk") == 0) {
    set_extra_bit(o, PCRE2_EXTRA_ALLOW_LOOKAROUND_BSK, on(k));
#endif
  } else {
    error("Unknown PCRE option \"%s\".\n", k);
  }
  return 0;
}

static void apply_mapping_flags(pcre_options* o, mapping_t* map) {
  mapTraverse(map, apply_one_option, o);
}

static void apply_options_svalue(pcre_options* o, svalue_t* sv) {
  if (sv->type == T_NUMBER) {
    apply_int_flags(o, sv->u.number);
  } else if (sv->type == T_MAPPING) {
    apply_mapping_flags(o, sv->u.map);
  } else {
    error("Bad PCRE options (expected int or mapping).\n");
  }
}

static void assign_run_options(pcre_t* run, const pcre_options& o) {
  run->compile_flags = o.compile_flags;
  run->extra_options = o.extra_options;
  run->newline = o.newline;
  run->bsr = o.bsr;
  run->exec_flags = o.exec_flags;
  run->substitute_flags = o.substitute_flags;
  run->match_limit = o.match_limit;
  run->depth_limit = o.depth_limit;
  run->heap_limit = o.heap_limit;
  run->offset_limit = o.offset_limit;
  run->start_offset = o.start_offset;
  run->has_match_limit = o.has_match_limit;
  run->has_depth_limit = o.has_depth_limit;
  run->has_heap_limit = o.has_heap_limit;
  run->has_offset_limit = o.has_offset_limit;
  run->has_newline = o.has_newline;
  run->has_bsr = o.has_bsr;
  run->no_jit = o.no_jit;
}

static pcre2_match_context* pcre_run_context(pcre_t* p) {
  if (!p->has_match_limit && !p->has_depth_limit && !p->has_heap_limit && !p->has_offset_limit) {
    return nullptr;
  }
  if (p->mcontext == nullptr) {
    p->mcontext = pcre2_match_context_create(nullptr);
    if (p->mcontext == nullptr) {
      error("PCRE match context: out of memory\n");
    }
    if (p->has_match_limit) {
      pcre2_set_match_limit(p->mcontext, p->match_limit);
    }
    if (p->has_depth_limit) {
      pcre2_set_depth_limit(p->mcontext, p->depth_limit);
    }
#ifdef PCRE2_CONFIG_HEAPLIMIT
    if (p->has_heap_limit) {
      pcre2_set_heap_limit(p->mcontext, p->heap_limit);
    }
#endif
    if (p->has_offset_limit) {
      pcre2_set_offset_limit(p->mcontext, p->offset_limit);
    }
  }
  return p->mcontext;
}

static void pcre_throw_if_limit_error(int rc) {
  if (rc == PCRE2_ERROR_MATCHLIMIT) {
    error("PCRE match limit exceeded.\n");
  }
  if (rc == PCRE2_ERROR_DEPTHLIMIT) {
    error("PCRE depth limit exceeded.\n");
  }
#ifdef PCRE2_ERROR_HEAPLIMIT
  if (rc == PCRE2_ERROR_HEAPLIMIT) {
    error("PCRE heap limit exceeded.\n");
  }
#endif
#ifdef PCRE2_ERROR_BADOFFSET
  if (rc == PCRE2_ERROR_BADOFFSET) {
    error("PCRE option \"offset\" is past the end of the subject.\n");
  }
#endif
}

static pcre2_code* pcre_local_compile(pcre_t* p) {
  int errorcode = 0;
  PCRE2_SIZE erroffset = 0;
  pcre2_compile_context* ctx = nullptr;
  if (p->extra_options || p->has_newline || p->has_bsr) {
    ctx = pcre2_compile_context_create(nullptr);
    if (ctx == nullptr) {
      error("PCRE compile context: out of memory\n");
    }
    if (p->has_newline) {
      pcre2_set_newline(ctx, p->newline);
    }
    if (p->has_bsr) {
      pcre2_set_bsr(ctx, p->bsr);
    }
    if (p->extra_options) {
      pcre2_set_compile_extra_options(ctx, p->extra_options);
    }
  }
  p->re = pcre2_compile((PCRE2_SPTR)p->pattern, PCRE2_ZERO_TERMINATED, p->compile_flags, &errorcode,
                        &erroffset, ctx);
  if (ctx != nullptr) {
    pcre2_compile_context_free(ctx);
  }
  p->erroffset = static_cast<int>(erroffset);
  if (p->re == nullptr) {
    pcre2_get_error_message(errorcode, reinterpret_cast<PCRE2_UCHAR*>(p->error), sizeof(p->error));
    return nullptr;
  }
  p->error[0] = '\0';
#if !defined(__EMSCRIPTEN__)
  if (!p->no_jit) {
    pcre2_jit_compile(p->re, PCRE2_JIT_COMPLETE);
  }
#endif
  return p->re;
}

static int pcre_ensure_compiled(pcre_t* p) {
  p->re = pcre_get_cached_pattern(&pcre_cache, p);
  if (p->re == nullptr) {
    pcre_local_compile(p);
    if (p->re != nullptr) {
      pcre_cache_pattern(&pcre_cache, p->re, p);
    }
  }
  return p->re == nullptr ? -1 : 1;
}

static char* pcre_substitute_template(pcre_t* run, const char* repl, size_t repl_len) {
  uint32_t options = run->exec_flags | run->substitute_flags | PCRE2_SUBSTITUTE_OVERFLOW_LENGTH;
  pcre2_match_data* md = pcre2_match_data_create_from_pattern(run->re, nullptr);
  if (md == nullptr) {
    error("PCRE substitute failed: out of memory\n");
  }
  pcre2_match_context* ctx = pcre_run_context(run);
  PCRE2_SIZE outlen = run->s_length + 16;
  auto* out = static_cast<PCRE2_UCHAR*>(DMALLOC(outlen + 1, TAG_TEMPORARY, "pcre_substitute"));
  int rc = pcre2_substitute(run->re, (PCRE2_SPTR)run->subject, run->s_length, run->start_offset,
                            options, md, ctx, (PCRE2_SPTR)repl, repl_len, out, &outlen);
  if (rc == PCRE2_ERROR_NOMEMORY) {
    FREE(out);
    out = static_cast<PCRE2_UCHAR*>(DMALLOC(outlen + 1, TAG_TEMPORARY, "pcre_substitute"));
    rc = pcre2_substitute(run->re, (PCRE2_SPTR)run->subject, run->s_length, run->start_offset,
                          options, md, ctx, (PCRE2_SPTR)repl, repl_len, out, &outlen);
  }
  pcre2_match_data_free(md);
  if (rc < 0) {
    char err[256];
    pcre2_get_error_message(rc, reinterpret_cast<PCRE2_UCHAR*>(err), sizeof(err));
    FREE(out);
    error("PCRE substitute failed: %s\n", err);
  }
  char* ret = new_string(static_cast<int>(outlen), "pcre_substitute");
  memcpy(ret, out, outlen);
  ret[outlen] = '\0';
  FREE(out);
  return ret;
}

static int pcre_exec_at(pcre_t* p, size_t offset, int extra_flags) {
  pcre2_match_data* md = pcre2_match_data_create_from_pattern(p->re, nullptr);
  if (md == nullptr) {
    return PCRE2_ERROR_NOMEMORY;
  }
  int rc = pcre2_match(p->re, (PCRE2_SPTR)p->subject, p->s_length, offset,
                       p->exec_flags | extra_flags, md, pcre_run_context(p));
  if (rc >= 0) {
    PCRE2_SIZE* ov = pcre2_get_ovector_pointer(md);
    uint32_t n = pcre2_get_ovector_count(md);
    int need = static_cast<int>(n * 2);
    if (p->ovector == nullptr || p->ovecsize < need) {
      if (p->ovector) {
        FREE(p->ovector);
      }
      p->ovector = (int*)DCALLOC(need, sizeof(int), TAG_TEMPORARY, "pcre_exec_at");
      p->ovecsize = need;
    }
    for (uint32_t i = 0; i < n; i++) {
      p->ovector[2 * i] = static_cast<int>(ov[2 * i]);
      p->ovector[2 * i + 1] = static_cast<int>(ov[2 * i + 1]);
    }
  }
  pcre2_match_data_free(md);
  return rc;
}

static int pcre_local_exec(pcre_t* p) {
  p->namecount = 0;
  p->name_entry_size = 0;
  p->name_table = nullptr;
  pcre2_pattern_info(p->re, PCRE2_INFO_NAMECOUNT, &p->namecount);
  pcre2_pattern_info(p->re, PCRE2_INFO_NAMEENTRYSIZE, &p->name_entry_size);
  pcre2_pattern_info(p->re, PCRE2_INFO_NAMETABLE, &p->name_table);

  p->rc = pcre_exec_at(p, p->start_offset, 0);
  pcre_throw_if_limit_error(p->rc);
  return p->rc;
}

static int pcre_magic(pcre_t* p) {
  if (pcre_ensure_compiled(p) < 0) {
    return -1;
  }
  pcre_local_exec(p);
  return 1;
}

static int pcre_query_match(pcre_t* p) {
  if (p->rc >= 0 || p->rc == PCRE2_ERROR_PARTIAL) {
    return 1;
  }
  return 0;
}

auto pcre_match_all(const char* subject, size_t subject_len, const char* pattern,
                    const pcre_options& o) {
  pcre_t* run;

  run = (pcre_t*)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "pcre_match_single : run");
  run->ovector = nullptr;
  run->ovecsize = 0;
  run->pattern = pattern;
  run->subject = subject;
  run->s_length = subject_len;
  assign_run_options(run, o);

  DEFER { pcre_free_memory(run); };

  run->re = pcre_get_cached_pattern(&pcre_cache, run);

  if (run->re == nullptr) {
    pcre_local_compile(run);
    pcre_cache_pattern(&pcre_cache, run->re, run);
  }

  if (run->re == nullptr) {
    error("PCRE compilation failed at offset %d: %s\n", run->erroffset, run->error);
  }

  std::vector<std::vector<svalue_t>> matches;

  int rc = 0;
  size_t offset = run->start_offset;
  int retry_flags = 0;
  while (offset < run->s_length) {
    rc = pcre_exec_at(run, offset, retry_flags);
    pcre_throw_if_limit_error(rc);
    if (rc < 0) {
      if (retry_flags == 0) {
        break;
      }
      // The previous match was empty and nothing non-empty matches at this
      // position: advance one UTF-8 character and continue scanning.
      offset++;
      while (offset < run->s_length && (run->subject[offset] & 0xC0) == 0x80) {
        offset++;
      }
      retry_flags = 0;
      continue;
    }
    std::vector<svalue_t> match;
    for (int i = 0; i < rc; ++i) {
      unsigned int start, length;
      length = run->ovector[2 * i + 1] - run->ovector[2 * i];
      start = run->ovector[2 * i];

      char* match_str = new_string(length, "pcre get substrings");
      snprintf(match_str, length + 1, "%.*s", length, run->subject + start);
      svalue_t const item = {
          .type = T_STRING,
          .subtype = STRING_MALLOC,
          .u = {.string = match_str},
      };
      match.push_back(item);
    }
    matches.push_back(match);
    // An empty match would rescan the same offset forever; require the next
    // match at this position to be non-empty (the standard pcredemo idiom).
    retry_flags = (run->ovector[1] == run->ovector[0]) ? (PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED) : 0;
    offset = static_cast<size_t>(run->ovector[1]);
  }

  return matches;
}

static int pcre_match_single(svalue_t* str, const char* pattern, const pcre_options& o) {
  pcre_t* run;
  int ret;

  run = (pcre_t*)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "pcre_match_single : run");
  run->ovector = nullptr;
  run->ovecsize = 0;
  run->pattern = pattern;
  run->subject = str->u.string;
  run->s_length = SVALUE_STRLEN(str);
  assign_run_options(run, o);

  DEFER { pcre_free_memory(run); };

  if (pcre_magic(run) < 0) {
    error("PCRE compilation failed at offset %d: %s\n", run->erroffset, run->error);
  }

  ret = pcre_query_match(run);

  return ret;
}

static array_t* pcre_match(array_t* v, const char* pattern, int flag, const pcre_options& o) {
  pcre_t* run;
  array_t* ret;
  svalue_t *sv1, *sv2;
  char* res;
  int num_match, size, match = !(flag & 2);

  if (!(size = v->size)) {
    return &the_null_array;
  }

  run = (pcre_t*)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "pcre_match : run");
  run->ovector = nullptr;
  run->ovecsize = 0;
  run->pattern = pattern;
  assign_run_options(run, o);

  run->re = pcre_get_cached_pattern(&pcre_cache, run);

  DEFER { pcre_free_memory(run); };

  if (run->re == nullptr) {
    if (pcre_local_compile(run) == nullptr) {
      const char* rerror = run->error;
      int const offset = run->erroffset;

      error("PCRE compilation failed at offset %d: %s\n", offset, rerror);
    } else {
      pcre_cache_pattern(&pcre_cache, run->re, run);
    }
  }

  res = (char*)DMALLOC(size, TAG_TEMPORARY, "prcre_match: res");
  DEFER { FREE(res); };
  sv1 = v->item + size;
  num_match = 0;

  while (size--) {
    if ((--sv1)->type != T_STRING) {
      res[size] = 0;
      continue;
    }

    run->subject = sv1->u.string;
    run->s_length = SVALUE_STRLEN(sv1);

    pcre_local_exec(run);

    if (pcre_query_match(run) != match) {  // was not checking for match! (woom)
      res[size] = 0;
      continue;
    }

    res[size] = 1;
    num_match++;
  }

  flag &= 1;
  ret = allocate_empty_array(num_match << flag);
  sv2 = ret->item + (num_match << flag);
  size = v->size;

  while (size--) {
    if (res[size]) {
      if (flag) {
        (--sv2)->type = T_NUMBER;
        sv2->u.number = size + 1;
      }

      (--sv2)->type = T_STRING;

      sv1 = v->item + size;
      *sv2 = *sv1;

      if (sv1->subtype & STRING_COUNTED) {
        INC_COUNTED_REF(sv1->u.string);
        md_record_ref_journal(PTR_TO_NODET(sv1->u.string), true, MSTR_REF(sv1->u.string),
                              __CURRENT_FILE_LINE__);
        ADD_STRING(MSTR_SIZE(sv1->u.string));
      }

      if (!--num_match) {
        break;
      }
    }
  }

  return ret;
}

/* This is mostly copy/paste from reg_assoc, some parts are changed
 * TODO: rewrite with new logic
 */
static array_t* pcre_assoc(svalue_t* str, array_t* pat, array_t* tok, svalue_t* def,
                           const pcre_options& o) {
  int i;
  size_t size;
  const char* tmp;
  array_t* ret;

  if ((size = pat->size) != tok->size) {
    error("Pattern and token array size must be identical.\n");
  }

  for (i = 0; i < size; i++)
    if (pat->item[i].type != T_STRING) {
      error("Non-string found in pattern array.\n");
    }

  ret = allocate_empty_array(2);

  if (size) {
    pcre_t** rgpp;
    struct RegMatch {
      int tok_i;
      const char *begin, *end;
      struct RegMatch* next;
    } *rmp = (struct RegMatch*)nullptr, *rmph = (struct RegMatch*)nullptr;
    int num_match = 0, length;
    svalue_t *sv1, *sv2, *sv;
    int regindex;
    pcre_t* tmpreg;
    int laststart;

    rgpp = (pcre_t**)DCALLOC(size, sizeof(pcre_t*), TAG_TEMPORARY, "pcre_assoc : rgpp");

    for (i = 0; i < size; i++) {
      rgpp[i] = (pcre_t*)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "pcre_assoc : rgpp[i]");
      rgpp[i]->ovector = nullptr;
      rgpp[i]->ovecsize = 0;
      rgpp[i]->pattern = pat->item[i].u.string;
      assign_run_options(rgpp[i], o);
      rgpp[i]->re = pcre_get_cached_pattern(&pcre_cache, rgpp[i]);

      if (rgpp[i]->re == nullptr) {
        if (pcre_local_compile(rgpp[i]) == nullptr) {
          const char* rerror = rgpp[i]->error;
          int const offset = rgpp[i]->erroffset;

          pcre_free_memory(rgpp[i]);
          while (i--) {
            pcre_free_memory(rgpp[i]);
          }

          FREE(rgpp);
          free_empty_array(ret);
          error("PCRE compilation failed at offset %d: %s\n", offset, rerror);
        } else {
          pcre_cache_pattern(&pcre_cache, rgpp[i]->re, rgpp[i]);
        }
      }
    }

    tmp = str->u.string;
    int const totalsize = SVALUE_STRLEN(str);
    int used = 0;
    while (*tmp) {
      laststart = 0;
      regindex = -1;

      for (i = 0; i < size; i++) {
        rgpp[i]->subject = tmp;
        rgpp[i]->s_length = totalsize - used;

        pcre_local_exec(tmpreg = rgpp[i]);

        if (pcre_query_match(tmpreg)) {
          size_t curr_temp_sz;

          curr_temp_sz = totalsize - used - tmpreg->ovector[0];
          if (!tmpreg->ovector[0]) {
            regindex = i;
            break;
          }
          if (laststart < curr_temp_sz) {
            laststart = curr_temp_sz;
            regindex = i;
          }
        }
      }

      if (regindex >= 0) {
        const char *rmpb_tmp, *rmpe_tmp;
        num_match++;

        if (rmp) {
          rmp->next = (struct RegMatch*)DMALLOC(sizeof(struct RegMatch), TAG_TEMPORARY,
                                                "pcre_assoc : rmp->next");
          rmp = rmp->next;
        } else
          rmph = rmp =
              (struct RegMatch*)DMALLOC(sizeof(struct RegMatch), TAG_TEMPORARY, "pcre_assoc : rmp");

        tmpreg = rgpp[regindex];

        rmpb_tmp = tmp + tmpreg->ovector[0];
        rmpe_tmp = tmp + tmpreg->ovector[1];

        rmp->begin = rmpb_tmp;
        rmp->end = tmp = rmpe_tmp;
        used += tmpreg->ovector[1];
        rmp->tok_i = regindex;
        rmp->next = (struct RegMatch*)nullptr;
      } else {
        break;
      }

      if (rmp->begin == tmp && (!*++tmp)) {
        break;
      }
    }

    // Validate the result array size BEFORE allocating it: allocate_empty_array()
    // itself error()s past this limit, which would unwind past rgpp/rmph/ret
    // (none of which are RAII-guarded here) and leak them all.
    if (2 * num_match + 1 > CONFIG_INT(__MAX_ARRAY_SIZE__)) {
      for (i = 0; i < size; i++) {
        pcre_free_memory(rgpp[i]);
      }
      FREE(rgpp);
      while ((rmp = rmph)) {
        rmph = rmp->next;
        FREE((char*)rmp);
      }
      free_empty_array(ret);
      error("Too many matches (%d) for pcre_assoc().\n", num_match);
    }

    sv = ret->item;
    sv->type = T_ARRAY;
    sv1 = (sv->u.arr = allocate_empty_array(2 * num_match + 1))->item;

    sv++;
    sv->type = T_ARRAY;
    sv2 = (sv->u.arr = allocate_empty_array(2 * num_match + 1))->item;

    rmp = rmph;

    tmp = str->u.string;

    while (num_match--) {
      char* svtmp;
      length = rmp->begin - tmp;

      sv1->type = T_STRING;
      sv1->subtype = STRING_MALLOC;
      sv1->u.string = svtmp = new_string(length, "pcre_assoc : sv1");
      strncpy(svtmp, tmp, length);
      svtmp[length] = 0;

      sv1++;
      assign_svalue_no_free(sv2++, def);
      tmp += length;

      length = rmp->end - rmp->begin;

      sv1->type = T_STRING;
      sv1->subtype = STRING_MALLOC;
      sv1->u.string = svtmp = new_string(length, "pcre_assoc : sv1");
      strncpy(svtmp, tmp, length);
      svtmp[length] = 0;

      sv1++;
      assign_svalue_no_free(sv2++, &tok->item[rmp->tok_i]);
      tmp += length;
      rmp = rmp->next;
    }

    sv1->type = T_STRING;
    sv1->subtype = STRING_MALLOC;
    sv1->u.string = string_copy(tmp, "pcre_assoc");
    assign_svalue_no_free(sv2, def);

    for (i = 0; i < size; i++) {
      pcre_free_memory(rgpp[i]);
    }

    FREE(rgpp);

    while ((rmp = rmph)) {
      rmph = rmp->next;
      FREE((char*)rmp);
    }
    return ret;
  }
  svalue_t* temp;
  svalue_t* sv;

  (sv = ret->item)->type = T_ARRAY;
  temp = (sv->u.arr = allocate_empty_array(1))->item;
  assign_svalue_no_free(temp, str);
  sv = &ret->item[1];
  sv->type = T_ARRAY;
  assign_svalue_no_free((sv->u.arr = allocate_empty_array(1))->item, def);
  return ret;
}

static array_t* pcre_get_substrings(pcre_t* run, bool include_names) {
  array_t* ret;
  unsigned int i;

  unsigned int const base_size = run->rc > 0 ? static_cast<unsigned int>(run->rc - 1) : 0;
  bool const add_names = include_names;
  unsigned int const ret_size = base_size + (add_names ? 1 : 0);

  ret = allocate_empty_array(ret_size);

  if (run->rc != 1) {
    for (i = 1; i <= base_size; i++) {
      unsigned int start, length;
      /* Allocate enough for the match */
      length = run->ovector[2 * i + 1] - run->ovector[2 * i];
      start = run->ovector[2 * i];
      char* match = new_string(length, "pcre get substrings");

      sprintf(match, "%.*s", length, run->subject + start);
      ret->item[i - 1].type = T_STRING;
      ret->item[i - 1].subtype = STRING_MALLOC;
      ret->item[i - 1].u.string = match;
    }
  }

  if (add_names && ret_size > 0) {
    mapping_t* map = allocate_mapping(run->namecount);

    if (run->namecount > 0) {
      for (int name_index = 0; name_index < run->namecount; name_index++) {
        auto* entry = run->name_table + name_index * run->name_entry_size;
        int const group = (entry[0] << 8) | entry[1];

        if (group <= 0 || static_cast<unsigned int>(group) > base_size) {
          continue;  // skip overall match or groups not captured in ret
        }

        int const ovec_index = group * 2;
        if (ovec_index + 1 >= run->ovecsize) {
          continue;
        }
        if (run->ovector[ovec_index] < 0 || run->ovector[ovec_index + 1] < 0) {
          continue;  // group did not participate
        }

        svalue_t key;
        key.type = T_STRING;
        key.subtype = STRING_SHARED;
        key.u.string = make_shared_string(reinterpret_cast<const char*>(entry + 2));

        svalue_t* slot = find_for_insert(map, &key, 1);
        free_string(key.u.string);

        if (slot != nullptr) {
          assign_svalue_no_free(slot, &ret->item[group - 1]);
        }
      }
    }

    ret->item[ret_size - 1].type = T_MAPPING;
    ret->item[ret_size - 1].subtype = 0;
    ret->item[ret_size - 1].u.map = map;
  }

  return ret;
}

static char* pcre_get_replace(pcre_t* run, array_t* replacements) {
  unsigned int ret_pos = 0, i;
  size_t ret_sz;
  char* ret;

  /* Set size of return string to subject length */
  ret_sz = run->s_length;

  /* Subtract total size of all substrings */
  int prev = run->ovector[2];
  for (i = 1; i <= (run->rc - 1); i++) {
    // printf("%d %d %d\n", ret_sz, run->ovector[2*i],run->ovector[2*i+1] );
    if (run->ovector[2 * i] >= prev) {
      ret_sz -= (size_t)(run->ovector[2 * i + 1] - run->ovector[2 * i]);
      prev = run->ovector[2 * i + 1];
      /* Add total size of all replacements */
      ret_sz += SVALUE_STRLEN(&replacements->item[i - 1]);
    }
  }

  /* Allocate space for the return string */
  ret = new_string((ret_sz), "pcre get replace");
  // printf("ret_sz:%d\n", ret_sz);

  if (run->rc <= 1) {
    memcpy(ret, run->subject, ret_sz);
    *(ret + ret_sz) = '\0';
    return ret;
  }

  // Copy the subject, replacing each SELECTED capture group with its
  // replacement. The selection MUST match the size loop above exactly
  // (a group is selected only when it starts at/after the end of the last
  // selected group -- i.e. non-nested/non-overlapping); otherwise the copied
  // length diverges from ret_sz and overflows ret. Every write is also
  // clamped to the remaining space as defense in depth.
  prev = run->ovector[2];  // reuse: gate -- next selected group must start >= this
  int last_end = run->ovector[2];
  ret_pos = 0;

  // Copy the subject prefix up to the first potential group start.
  {
    size_t n = (size_t)run->ovector[2];
    if (n > ret_sz - ret_pos) n = ret_sz - ret_pos;
    memcpy(ret + ret_pos, run->subject, n);
    ret_pos += n;
  }

  for (i = 1; i <= (run->rc - 1); i++) {
    int gstart = run->ovector[2 * i];
    int gend = run->ovector[2 * i + 1];

    if (gstart < prev) {
      continue;  // nested/overlapping group -- skipped by the size loop too
    }

    const char* rep = replacements->item[i - 1].u.string;
    size_t rep_sz = SVALUE_STRLEN(&replacements->item[i - 1]);

    // Gap of subject text between the previous selected group and this one.
    if (gstart > last_end) {
      size_t gap = (size_t)(gstart - last_end);
      if (gap > ret_sz - ret_pos) gap = ret_sz - ret_pos;
      memcpy(ret + ret_pos, run->subject + last_end, gap);
      ret_pos += gap;
    }

    // The replacement for this group.
    if (rep_sz > ret_sz - ret_pos) rep_sz = ret_sz - ret_pos;
    memcpy(ret + ret_pos, rep, rep_sz);
    ret_pos += rep_sz;

    last_end = gend;
    prev = gend;
  }

  // Copy the trailing subject text after the last selected group.
  if ((size_t)last_end < run->s_length) {
    size_t tail = run->s_length - (size_t)last_end;
    if (tail > ret_sz - ret_pos) tail = ret_sz - ret_pos;
    memcpy(ret + ret_pos, run->subject + last_end, tail);
    ret_pos += tail;
  }

  *(ret + ret_pos) = '\0';

  return ret;
}

static void pcre_free_memory(pcre_t* p) {
  if (p->ovector) {
    FREE(p->ovector);
  }
  if (p->mcontext) {
    pcre2_match_context_free(p->mcontext);
  }
  FREE(p);
}

static int pcre_same_compile(const pcre_t* p, const struct pcre_cache_bucket_t* node) {
  return node->compile_flags == p->compile_flags && node->extra_options == p->extra_options &&
         node->newline == p->newline && node->bsr == p->bsr && node->no_jit == p->no_jit;
}

static unsigned pcre_cache_bucket_of(const char* shared_pattern, const pcre_t* p) {
  unsigned h = HASH(BLOCK(shared_pattern)) ^ p->compile_flags ^ p->extra_options ^ p->newline ^ p->bsr;
  if (p->no_jit) {
    h ^= 0x9e3779b9u;
  }
  return h % PCRE_CACHE_SIZE;
}

// Caching functions, add new ones at the front of the bucket so we find them
// faster
static int pcre_cache_pattern(struct pcre_cache_t* table, pcre2_code* cpat, const pcre_t* p) {
  const auto* shared_pattern = make_shared_string(p->pattern);
  unsigned int const bucket = pcre_cache_bucket_of(shared_pattern, p);
  PCRE2_SIZE sz = 0;
  struct pcre_cache_bucket_t* tmp;
  struct pcre_cache_bucket_t* node;
  int full;

  tmp = table->buckets[bucket];

  pcre2_pattern_info(cpat, PCRE2_INFO_SIZE, &sz);

  full = (pcrecachesize > 2 * PCRE_CACHE_SIZE);
  while (tmp) {
    if (shared_pattern == tmp->pattern && pcre_same_compile(p, tmp)) {
      break;
    }

    tmp = tmp->next;
  }

  if (tmp) {
    // does this even make sense? same pattern will always compile the same way?
    pcre2_code_free(tmp->compiled_pattern);
    node = tmp;
  } else {
    node = (struct pcre_cache_bucket_t*)DCALLOC(1, sizeof(struct pcre_cache_bucket_t),
                                                TAG_PCRE_CACHE, "pcre_cache_pattern : node");
    if (node == nullptr) {
      return -1;
    }
    if (!full) {
      pcrecachesize++;
    } else {
      if ((tmp = table->buckets[bucket])) {
        while (tmp->next) {
          tmp = tmp->next;
        }
        if (tmp == table->buckets[bucket]) {  // if the hash version works, most
                                              // of the time
          pcre2_code_free(tmp->compiled_pattern);
          free_string(tmp->pattern);
          FREE(tmp);
          table->buckets[bucket] = nullptr;
        } else {
          struct pcre_cache_bucket_t* tmp2;
          tmp2 = table->buckets[bucket];
          while (tmp2->next != tmp) {
            tmp2 = tmp2->next;  // shouldn't get here often
          }
          pcre2_code_free(tmp->compiled_pattern);
          free_string(tmp->pattern);
          FREE(tmp);
          tmp2->next = nullptr;
        }
      }
    }
    node->next = table->buckets[bucket];
    table->buckets[bucket] = node;
  }

  node->pattern = shared_pattern;
  node->compile_flags = p->compile_flags;
  node->extra_options = p->extra_options;
  node->newline = p->newline;
  node->bsr = p->bsr;
  node->no_jit = p->no_jit;
  node->compiled_pattern = cpat;
  node->size = static_cast<int>(sz);

  return 0;
}

static pcre2_code* pcre_get_cached_pattern(struct pcre_cache_t* table, const pcre_t* p) {
  const auto* shared_pattern = make_shared_string(p->pattern);
  unsigned int const bucket = pcre_cache_bucket_of(shared_pattern, p);
  struct pcre_cache_bucket_t* node;
  struct pcre_cache_bucket_t* lnode = nullptr;
  node = table->buckets[bucket];

  while (node) {
    if (shared_pattern == node->pattern && pcre_same_compile(p, node)) {
      if (node != table->buckets[bucket]) {
        // not at the front, move it there, so the most used pattern is fastest
        lnode->next = node->next;
        node->next = table->buckets[bucket];
        table->buckets[bucket] = node;
      }
      free_string(shared_pattern);
      return node->compiled_pattern;
    }
    lnode = node;
    node = node->next;
  }
  free_string(shared_pattern);
  return nullptr;
}

static mapping_t* pcre_get_cache() {
  int size = 0, i;
  mapping_t* ret;
  struct pcre_cache_bucket_t* node;

  // Calculate size for mapping
  for (i = 0; i < PCRE_CACHE_SIZE; i++) {
    if (pcre_cache.buckets[i] != nullptr) {
      node = pcre_cache.buckets[i];

      while (node) {
        node = node->next;
        size++;
      }
    }
  }

  ret = allocate_mapping(size);

  for (i = 0; i < PCRE_CACHE_SIZE; i++) {
    if (pcre_cache.buckets[i] != nullptr) {
      node = pcre_cache.buckets[i];

      while (node) {
        size_t keylen = strlen(node->pattern) + 64;
        char* key = (char*)DMALLOC(keylen, TAG_TEMPORARY, "pcre_cache key");
        snprintf(key, keylen, "%s|0x%x|0x%x|n%u|b%u|j%d", node->pattern, node->compile_flags,
                 node->extra_options, node->newline, node->bsr, node->no_jit);
        add_mapping_pair(ret, key, node->size);
        FREE(key);
        node = node->next;
      }
    }
  }
  return ret;
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_pcre_cache() {
  for (auto& bucket : pcre_cache.buckets) {
    if (bucket != nullptr) {
      auto* node = bucket;
      while (node != nullptr) {
        DO_MARK(node, TAG_PCRE_CACHE);
        EXTRA_REF(BLOCK(node->pattern))++;
        node = node->next;
      }
    }
  }
}
#endif

void f_pcre_match_all() {
  array_t* v;
  pcre_options opts = default_pcre_options();

  if (st_num_arg >= 3) {
    apply_options_svalue(&opts, sp);
    pop_stack();
    st_num_arg--;
  }

  const auto* pattern = (sp)->u.string;
  const auto* subject = (sp - 1)->u.string;
  auto subject_len = SVALUE_STRLEN(sp - 1);

  auto matches = pcre_match_all(subject, subject_len, pattern, opts);

  pop_2_elems();

  v = allocate_array(matches.size());
  for (int i = 0; i < matches.size(); i++) {
    auto& match = matches[i];
    auto* match_array = allocate_array(match.size());
    v->item[i].type = T_ARRAY;
    v->item[i].u.arr = match_array;
    for (int j = 0; j < match.size(); j++) {
      match_array->item[j] = match[j];
    }
  }

  push_refed_array(v);
}
