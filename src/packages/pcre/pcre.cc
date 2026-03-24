/* FILES
 *    - pcre.c
 *    - pcre_spec.c
 *    - pcre.h
 *
 * PCRE (Perl Compatible Regular Expressions)
 *    Efuns using system library of PCRE (http://www.pcre.org/)
 *    The PCRE library was created by Philip Hazel at the
 *    University of Cambridge
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
 *         - returns the version of the compiled PCRE library used
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
// study pattern
// extra options when matching, greedy, lazy, possesive, etc.
// match_all ? see previous
// store reg->error & reg->erroffset before error(..)

#include <thirdparty/scope_guard/scope_guard.hpp>
#include "base/package_api.h"

#include "pcre.h"
#include "include/pcre_flags.h"
#include "vm/internal/base/mapping.h"

// Prototype declarations
static void pcre_free_memory(pcre_t *p);
static pcre *pcre_local_compile(pcre_t *p);
static int pcre_local_exec(pcre_t *p);
static int pcre_magic(pcre_t *p);
static int pcre_query_match(pcre_t *p);
static inline int compute_compile_options(int flags);
static inline int compute_exec_options(int flags);
static int pcre_match_single(svalue_t *str, const char *pattern, int pcre_flags);
static array_t *pcre_match(array_t *v, const char *pattern, int flag, int pcre_flags);
static array_t *pcre_assoc(svalue_t *str, array_t *pat, array_t *tok, svalue_t *def, int pcre_flags);
static char *pcre_get_replace(pcre_t *run, array_t *replacements);
static array_t *pcre_get_substrings(pcre_t *run, bool include_names);
// Caching functions
static int pcre_cache_pattern(struct pcre_cache_t *table, pcre *cpat, const char *pattern, int compile_flags);
static pcre *pcre_get_cached_pattern(struct pcre_cache_t *table, const char *pattern, int compile_flags);
static mapping_t *pcre_get_cache();
int pcrecachesize = 0;
// Globals
struct pcre_cache_t pcre_cache = {{nullptr}};

// efuns
void f_pcre_version() {
  char *version;
  version = (char *)pcre_version();
  push_constant_string(version);
}

void f_pcre_match() {
  array_t *v;
  int flag = 0;
  int pcre_flags = 0;
  bool is_string = ((sp - 1)->type == T_STRING);

  // optional 4th arg: pcre_flags
  if (st_num_arg > 3) {
    if (sp->type != T_NUMBER) {
      error("Bad argument 4 to pcre_match()\n");
    }
    pcre_flags = (sp--)->u.number;
    st_num_arg--;
  }

  // optional 3rd arg:
  if (st_num_arg == 3) {
    if (sp->type != T_NUMBER) {
      error("Bad argument 3 to pcre_match()\n");
    }
    if (is_string) {
      pcre_flags = (sp--)->u.number;
    } else {
      flag = (sp--)->u.number;
    }
    st_num_arg--;
  }

  if (is_string) {
    flag = pcre_match_single((sp - 1), sp->u.string, pcre_flags);

    free_string_svalue(sp--);
    free_string_svalue(sp);
    put_number(flag);
  } else {
    v = pcre_match((sp - 1)->u.arr, sp->u.string, flag, pcre_flags);

    free_string_svalue(sp--);
    free_array(sp->u.arr);
    sp->u.arr = v;
  }
}

void f_pcre_assoc() {
  svalue_t *arg;
  array_t *vec;
  int pcre_flags = 0;

  if (st_num_arg == 5) {
    if (sp->type != T_NUMBER) {
      error("Bad argument 5 to pcre_assoc()\n");
    }
    pcre_flags = sp->u.number;
    sp--;
    st_num_arg--;
  }

  arg = sp - st_num_arg + 1;

  if ((arg + 2)->type != T_ARRAY) {
    error("Bad argument 3 to pcre_assoc()\n");
  }

  vec = pcre_assoc(arg, (arg + 1)->u.arr, (arg + 2)->u.arr, st_num_arg > 3 ? (arg + 3) : &const0, pcre_flags);

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
  pcre_t *run;
  array_t *ret;
  int include_names = 0;
  int pcre_flags = 0;

  if (st_num_arg >= 4) {
    if (sp->type != T_NUMBER) {
      error("Bad argument 4 to pcre_extract()\n");
    }
    pcre_flags = sp->u.number;
    sp--;
    st_num_arg--;
  }

  svalue_t *arg = sp - st_num_arg + 1;

  if (st_num_arg == 3) {
    if ((arg + 2)->type != T_NUMBER) {
      error("Bad argument 3 to pcre_extract()\n");
    }
    include_names = (arg + 2)->u.number != 0;
  } else if (st_num_arg != 2) {
    error("pcre_extract() requires 2 or 3 arguments\n");
  }

  run = (pcre_t *)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "f_pcre_extract : run");
  run->pattern = (arg + 1)->u.string;
  run->subject = arg->u.string;
  run->s_length = SVALUE_STRLEN(arg);
  run->ovector = nullptr;
  run->ovecsize = 0;
  run->compile_flags = compute_compile_options(pcre_flags);
  run->exec_flags = compute_exec_options(pcre_flags);
  DEFER { pcre_free_memory(run); };

  if (pcre_magic(run) < 0) {
    error("PCRE compilation failed at offset %d: %s\n", run->erroffset, run->error);
  }

  if (run->rc < 0) { /* No match. could do handling of matching errors if wanted */
    pop_n_elems(st_num_arg);
    push_refed_array(&the_null_array);
    return;
  }
  if (run->rc > (run->ovecsize / 3 - 1)) {
    error("Too many substrings.\n");
  }

  ret = pcre_get_substrings(run, include_names);
  pop_n_elems(st_num_arg);

  push_refed_array(ret);
}

void f_pcre_replace() {
  pcre_t *run;
  array_t *replacements;

  char *ret;
  int pcre_flags = 0;

  if (st_num_arg >= 4) {
    if (sp->type != T_NUMBER) {
      error("Bad argument 4 to pcre_replace()\n");
    }
    pcre_flags = (sp--)->u.number;
    st_num_arg--;
  }

  run = (pcre_t *)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "f_pcre_replace: run");

  run->ovector = nullptr;
  run->ovecsize = 0;
  run->pattern = (sp - 1)->u.string;
  run->subject = (sp - 2)->u.string;
  replacements = sp->u.arr;

  run->s_length = SVALUE_STRLEN(sp - 2);
  run->compile_flags = compute_compile_options(pcre_flags);
  run->exec_flags = compute_exec_options(pcre_flags);
  DEFER { pcre_free_memory(run); };

  if (pcre_magic(run) < 0) {
    error("PCRE compilation failed at offset %d: %s\n", run->erroffset, run->error);
  }

  if (run->rc < 0) { /* No match. could do handling of matching errors if wanted */
    pop_2_elems();
    return;
  }

  if (run->rc > (run->ovecsize / 3 - 1)) {
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

  ret = pcre_get_replace(run, replacements);

  pop_3_elems();
  push_malloced_string(ret);
}

// string pcre_replace_callback(string, string, function)
void f_pcre_replace_callback() {
  int num_arg = st_num_arg, i;
  char *ret;
  pcre_t *run;
  svalue_t *arg;
  array_t *arr, *r;
  function_to_call_t ftc;
  int pcre_flags = 0;

  if (num_arg >= 4 && sp->type == T_NUMBER) {
    pcre_flags = sp->u.number;
    sp--;
    st_num_arg--;
    num_arg--;
  }

  arg = sp - num_arg + 1;

  run = (pcre_t *)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "f_pcre_replace: run");
  run->ovector = nullptr;
  run->ovecsize = 0;
  run->subject = arg->u.string;
  run->pattern = (arg + 1)->u.string;

  run->s_length = SVALUE_STRLEN(arg);
  run->compile_flags = compute_compile_options(pcre_flags);
  run->exec_flags = compute_exec_options(pcre_flags);
  DEFER { pcre_free_memory(run); };

  if (pcre_magic(run) < 0) {
    error("PCRE compilation failed at offset %d: %s\n", run->erroffset, run->error);
  }

  if (run->rc < 0) { /* No match. could do handling of matching errors if wanted */
    pop_n_elems(num_arg - 1);
    return;
  }

  if (run->rc > (run->ovecsize / 3 - 1)) {
    error("Too many substrings.\n");
  }

  arr = pcre_get_substrings(run, false);

  if (arg[2].type == T_FUNCTION || arg[2].type == T_STRING) {
    process_efun_callback(2, &ftc, F_PCRE_REPLACE_CALLBACK);
  } else {  // 0
    error("Illegal third argument (0) to pcre_replace_callback");
  }

  r = allocate_array(run->rc - 1);  // can't use the empty variant in case we error below

  push_refed_array(r);
  push_refed_array(arr);
  error_context_t econ;

  save_context(&econ);
  try {
    for (i = 0; i < run->rc - 1; i++) {
      svalue_t *v;
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
  } catch (const char *) {
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
  mapping_t *m = nullptr;
  m = pcre_get_cache();
  if (!m) {
    push_number(0);
  } else {
    push_refed_mapping(m);
  }
}

// Internal functions utilized by the efuns
static inline int compute_compile_options(int flags) {
  int opts = PCRE_UTF8;
  if (flags & PCRE_I) opts |= PCRE_CASELESS;
  if (flags & PCRE_M) opts |= PCRE_MULTILINE;
  if (flags & PCRE_S) opts |= PCRE_DOTALL;
  if (flags & PCRE_U) opts |= PCRE_UNGREEDY;
  if (flags & PCRE_X) opts |= PCRE_EXTENDED;
  return opts;
}

static inline int compute_exec_options(int flags) {
  int opts = 0;
  if (flags & PCRE_A) opts |= PCRE_ANCHORED;
  return opts;
}

static pcre *pcre_local_compile(pcre_t *p) {
  p->re = pcre_compile(p->pattern, p->compile_flags, &p->error, &p->erroffset, nullptr);

  return p->re;
}

static int pcre_local_exec(pcre_t *p) {
  int capture_count = 0;
  pcre_fullinfo(p->re, nullptr, PCRE_INFO_CAPTURECOUNT, &capture_count);
  capture_count += 2;
  capture_count *= 3;
  if (p->ovector) {
    FREE(p->ovector);
  }
  p->ovector = (int *)DCALLOC(capture_count + 1, sizeof(int), TAG_TEMPORARY,
                              "pcre_local_exec");  // too much, but who cares
  p->ovecsize = capture_count;

  p->namecount = 0;
  p->name_entry_size = 0;
  p->name_table = nullptr;
  pcre_fullinfo(p->re, nullptr, PCRE_INFO_NAMECOUNT, &p->namecount);
  pcre_fullinfo(p->re, nullptr, PCRE_INFO_NAMEENTRYSIZE, &p->name_entry_size);
  pcre_fullinfo(p->re, nullptr, PCRE_INFO_NAMETABLE, &p->name_table);

  p->rc = pcre_exec(p->re, nullptr, p->subject, p->s_length, 0, p->exec_flags, p->ovector, capture_count);

  return p->rc;
}

static int pcre_magic(pcre_t *p) {
  p->re = pcre_get_cached_pattern(&pcre_cache, p->pattern, p->compile_flags);

  if (p->re == nullptr) {
    pcre_local_compile(p);
    pcre_cache_pattern(&pcre_cache, p->re, p->pattern, p->compile_flags);
  }

  if (p->re == nullptr) {
    return -1;
  }

  /* Add support for studied patterns here */
  // if(p->study && !found)
  // {

  // }
  pcre_local_exec(p);

  return 1;
}

static int pcre_query_match(pcre_t *p) { return p->rc < 0 ? 0 : 1; }

auto pcre_match_all(const char *subject, size_t subject_len, const char *pattern, int pcre_flags) {
  pcre_t *run;

  run = (pcre_t *)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "pcre_match_single : run");
  run->ovector = nullptr;
  run->ovecsize = 0;
  run->pattern = pattern;
  run->subject = subject;
  run->s_length = subject_len;
  run->compile_flags = compute_compile_options(pcre_flags);
  run->exec_flags = compute_exec_options(pcre_flags);

  DEFER { pcre_free_memory(run); };

  run->re = pcre_get_cached_pattern(&pcre_cache, run->pattern, run->compile_flags);

  if (run->re == nullptr) {
    pcre_local_compile(run);
    pcre_cache_pattern(&pcre_cache, run->re, run->pattern, run->compile_flags);
  }

  if (run->re == nullptr) {
    error("PCRE compilation failed at offset %d: %s\n", run->erroffset, run->error);
  }

  {
    int size = 0;
    pcre_fullinfo(run->re, nullptr, PCRE_INFO_CAPTURECOUNT, &size);
    size += 2;
    size *= 3;
    if (run->ovector) {
      FREE(run->ovector);
    }
    run->ovector = (int *)DCALLOC(size + 1, sizeof(int), TAG_TEMPORARY,
                                  "pcre_local_exec");  // too much, but who cares
    run->ovecsize = size;
  }

  std::vector<std::vector<svalue_t>> matches;

  int rc = 0;
  int offset = 0;
  while (offset < run->s_length && (rc = pcre_exec(run->re, nullptr, run->subject, run->s_length,
                                                   offset, run->exec_flags, run->ovector, run->ovecsize)) >= 0) {
    std::vector<svalue_t> match;
    for (int i = 0; i < rc; ++i) {
      unsigned int start, length;
      length = run->ovector[2 * i + 1] - run->ovector[2 * i];
      start = run->ovector[2 * i];

      char *match_str = new_string(length, "pcre get substrings");
      snprintf(match_str, length + 1, "%.*s", length, run->subject + start);
      svalue_t const item = {
          .type = T_STRING,
          .subtype = STRING_MALLOC,
          .u = {.string = match_str},
      };
      match.push_back(item);
    }
    matches.push_back(match);
    offset = run->ovector[1];
  }

  return matches;
}

static int pcre_match_single(svalue_t *str, const char *pattern, int pcre_flags) {
  pcre_t *run;
  int ret;

  run = (pcre_t *)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "pcre_match_single : run");
  run->ovector = nullptr;
  run->ovecsize = 0;
  run->pattern = pattern;
  run->subject = str->u.string;
  run->s_length = SVALUE_STRLEN(str);
  run->compile_flags = compute_compile_options(pcre_flags);
  run->exec_flags = compute_exec_options(pcre_flags);

  DEFER { pcre_free_memory(run); };

  if (pcre_magic(run) < 0) {
    error("PCRE compilation failed at offset %d: %s\n", run->erroffset, run->error);
  }

  ret = pcre_query_match(run);

  return ret;
}

static array_t *pcre_match(array_t *v, const char *pattern, int flag, int pcre_flags) {
  pcre_t *run;
  array_t *ret;
  svalue_t *sv1, *sv2;
  char *res;
  int num_match, size, match = !(flag & 2);

  if (!(size = v->size)) {
    return &the_null_array;
  }

  run = (pcre_t *)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "pcre_match : run");
  run->ovector = nullptr;
  run->ovecsize = 0;
  run->pattern = pattern;
  run->compile_flags = compute_compile_options(pcre_flags);
  run->exec_flags = compute_exec_options(pcre_flags);

  run->re = pcre_get_cached_pattern(&pcre_cache, run->pattern, run->compile_flags);

  DEFER { pcre_free_memory(run); };

  if (run->re == nullptr) {
    if (pcre_local_compile(run) == nullptr) {
      const char *rerror = run->error;
      int const offset = run->erroffset;

      error("PCRE compilation failed at offset %d: %s\n", offset, rerror);
    } else {
      pcre_cache_pattern(&pcre_cache, run->re, run->pattern, run->compile_flags);
    }
  }

  res = (char *)DMALLOC(size, TAG_TEMPORARY, "prcre_match: res");
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
        md_record_ref_journal(PTR_TO_NODET(sv1->u.string), true, MSTR_REF(sv1->u.string), __CURRENT_FILE_LINE__);
        ADD_STRING(MSTR_SIZE(sv1->u.string));
      }

      if (!--num_match) {
        break;
      }
    }
  }
  FREE(res);

  return ret;
}

/* This is mostly copy/paste from reg_assoc, some parts are changed
 * TODO: rewrite with new logic
 */
static array_t *pcre_assoc(svalue_t *str, array_t *pat, array_t *tok, svalue_t *def, int pcre_flags) {
  int i;
  size_t size;
  const char *tmp;
  array_t *ret;

  if ((size = pat->size) != tok->size) {
    error("Pattern and token array size must be identical.\n");
  }

  for (i = 0; i < size; i++)
    if (pat->item[i].type != T_STRING) {
      error("Non-string found in pattern array.\n");
    }

  ret = allocate_empty_array(2);

  if (size) {
    pcre_t **rgpp;
    struct RegMatch {
      int tok_i;
      const char *begin, *end;
      struct RegMatch *next;
    } *rmp = (struct RegMatch *)nullptr, *rmph = (struct RegMatch *)nullptr;
    int num_match = 0, length;
    svalue_t *sv1, *sv2, *sv;
    int regindex;
    pcre_t *tmpreg;
    int laststart;

    rgpp = (pcre_t **)DCALLOC(size, sizeof(pcre_t *), TAG_TEMPORARY, "pcre_assoc : rgpp");

    for (i = 0; i < size; i++) {
      rgpp[i] = (pcre_t *)DCALLOC(1, sizeof(pcre_t), TAG_TEMPORARY, "pcre_assoc : rgpp[i]");
      rgpp[i]->ovector = nullptr;
      rgpp[i]->ovecsize = 0;
      rgpp[i]->pattern = pat->item[i].u.string;
      rgpp[i]->compile_flags = compute_compile_options(pcre_flags);
      rgpp[i]->exec_flags = compute_exec_options(pcre_flags);
      rgpp[i]->re = pcre_get_cached_pattern(&pcre_cache, rgpp[i]->pattern, rgpp[i]->compile_flags);

      if (rgpp[i]->re == nullptr) {
        if (pcre_local_compile(rgpp[i]) == nullptr) {
          const char *rerror = rgpp[i]->error;
          int const offset = rgpp[i]->erroffset;

          pcre_free_memory(rgpp[i]);
          while (i--) {
            pcre_free_memory(rgpp[i]);
          }

          FREE(rgpp);
          free_empty_array(ret);
          error("PCRE compilation failed at offset %d: %s\n", offset, rerror);
        } else {
          pcre_cache_pattern(&pcre_cache, rgpp[i]->re, rgpp[i]->pattern, rgpp[i]->compile_flags);
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
          rmp->next = (struct RegMatch *)DMALLOC(sizeof(struct RegMatch), TAG_TEMPORARY,
                                                 "pcre_assoc : rmp->next");
          rmp = rmp->next;
        } else
          rmph = rmp = (struct RegMatch *)DMALLOC(sizeof(struct RegMatch), TAG_TEMPORARY,
                                                  "pcre_assoc : rmp");

        tmpreg = rgpp[regindex];

        rmpb_tmp = tmp + tmpreg->ovector[0];
        rmpe_tmp = tmp + tmpreg->ovector[1];

        rmp->begin = rmpb_tmp;
        rmp->end = tmp = rmpe_tmp;
        used += tmpreg->ovector[1];
        rmp->tok_i = regindex;
        rmp->next = (struct RegMatch *)nullptr;
      } else {
        break;
      }

      if (rmp->begin == tmp && (!*++tmp)) {
        break;
      }
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
      char *svtmp;
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
      FREE((char *)rmp);
    }
    return ret;
  }
  svalue_t *temp;
  svalue_t *sv;

  (sv = ret->item)->type = T_ARRAY;
  temp = (sv->u.arr = allocate_empty_array(1))->item;
  assign_svalue_no_free(temp, str);
  sv = &ret->item[1];
  sv->type = T_ARRAY;
  assign_svalue_no_free((sv->u.arr = allocate_empty_array(1))->item, def);
  return ret;
}

static array_t *pcre_get_substrings(pcre_t *run, bool include_names) {
  array_t *ret;
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
      char *match = new_string(length, "pcre get substrings");

      sprintf(match, "%.*s", length, run->subject + start);
      ret->item[i - 1].type = T_STRING;
      ret->item[i - 1].subtype = STRING_MALLOC;
      ret->item[i - 1].u.string = match;
    }
  }

  if (add_names && ret_size > 0) {
    mapping_t *map = allocate_mapping(run->namecount);

    if (run->namecount > 0) {
      for (int name_index = 0; name_index < run->namecount; name_index++) {
        auto *entry = run->name_table + name_index * run->name_entry_size;
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
        key.u.string = make_shared_string(reinterpret_cast<char *>(entry + 2));

        svalue_t *slot = find_for_insert(map, &key, 1);
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

static char *pcre_get_replace(pcre_t *run, array_t *replacements) {
  const auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);

  unsigned int ret_pos = 0, i;
  size_t ret_sz;
  char *ret;

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

  /* Copy start of subject up until first match */
  if (run->rc > 1) {
    strncpy(ret, run->subject, run->ovector[2]);
    ret_pos = run->ovector[2];
  } else {
    strncpy(ret, run->subject, ret_sz);
  }

  for (i = 1; i <= (run->rc - 1); i++) {
    unsigned int end, len_nxt;
    const char *rep;
    size_t rep_sz;

    end = run->ovector[2 * i + 1];

    if (i == (run->rc - 1)) {
      len_nxt = run->s_length - end;
    } else {
      len_nxt = run->ovector[2 * i + 2] - end;
    }

    if (len_nxt > max_string_length) {
      continue;  // nested ()s
    }

    rep = replacements->item[i - 1].u.string;
    rep_sz = SVALUE_STRLEN(&replacements->item[i - 1]);

    /* Copy first substring into return variable */
    strncpy(ret + ret_pos, rep, rep_sz);

    /* increment position in return variable by replacement size */
    ret_pos += rep_sz;

    strncpy(ret + ret_pos, run->subject + end, len_nxt);

    ret_pos += len_nxt;
  }

  *(ret + ret_sz) = '\0';

  return ret;
}

static void pcre_free_memory(pcre_t *p) {
  if (p->ovector) {
    FREE(p->ovector);
  }
  FREE(p);
}

// Caching functions, add new ones at the front of the bucket so we find them
// faster
static int pcre_cache_pattern(struct pcre_cache_t *table, pcre *cpat,
                              const char *pattern, int compile_flags)  // must be shared string
{
  const auto *shared_pattern = make_shared_string(pattern);
  unsigned int const bucket = (HASH(BLOCK(shared_pattern)) ^ compile_flags) % PCRE_CACHE_SIZE;
  size_t sz;
  struct pcre_cache_bucket_t *tmp;
  struct pcre_cache_bucket_t *node;
  int full;

  tmp = table->buckets[bucket];

  // Calculate size of compiled pattern, require size_t!
  pcre_fullinfo(cpat, nullptr, PCRE_INFO_SIZE, &sz);

  full = (pcrecachesize > 2 * PCRE_CACHE_SIZE);
  while (tmp) {
    if (shared_pattern == tmp->pattern && tmp->compile_flags == compile_flags) {
      break;
    }

    tmp = tmp->next;
  }

  if (tmp) {
    // does this even make sense? same pattern will always compile the same way?
    pcre_free(tmp->compiled_pattern);
    node = tmp;
  } else {
    node = (struct pcre_cache_bucket_t *)DCALLOC(1, sizeof(struct pcre_cache_bucket_t),
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
          pcre_free(tmp->compiled_pattern);
          free_string(tmp->pattern);
          FREE(tmp);
          table->buckets[bucket] = nullptr;
        } else {
          struct pcre_cache_bucket_t *tmp2;
          tmp2 = table->buckets[bucket];
          while (tmp2->next != tmp) {
            tmp2 = tmp2->next;  // shouldn't get here often
          }
          pcre_free(tmp->compiled_pattern);
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
  node->compile_flags = compile_flags;
  node->compiled_pattern = cpat;
  node->size = sz;

  return 0;
}

static pcre *pcre_get_cached_pattern(struct pcre_cache_t *table, const char *pattern, int compile_flags) {
  const auto *shared_pattern = make_shared_string(pattern);
  unsigned int const bucket = (HASH(BLOCK(shared_pattern)) ^ compile_flags) % PCRE_CACHE_SIZE;
  struct pcre_cache_bucket_t *node;
  struct pcre_cache_bucket_t *lnode = nullptr;
  node = table->buckets[bucket];

  while (node) {
    if (shared_pattern == node->pattern && node->compile_flags == compile_flags) {
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

static mapping_t *pcre_get_cache() {
  int size = 0, i;
  mapping_t *ret;
  struct pcre_cache_bucket_t *node;

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
        size_t keylen = strlen(node->pattern) + 16;
        char *key = (char *)DMALLOC(keylen, TAG_TEMPORARY, "pcre_cache key");
        snprintf(key, keylen, "%s|0x%x", node->pattern, node->compile_flags);
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
  for (auto &bucket : pcre_cache.buckets) {
    if (bucket != nullptr) {
      auto *node = bucket;
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
  array_t *v;

  int pcre_flags = 0;
  if (st_num_arg >= 3) {
    if (sp->type != T_NUMBER) {
      error("Bad argument 3 to pcre_match_all()\n");
    }
    pcre_flags = (sp--)->u.number;
    st_num_arg--;
  }

  const auto *pattern = (sp)->u.string;
  const auto *subject = (sp - 1)->u.string;
  auto subject_len = SVALUE_STRLEN(sp - 1);

  auto matches = pcre_match_all(subject, subject_len, pattern, pcre_flags);

  pop_2_elems();

  v = allocate_array(matches.size());
  for (int i = 0; i < matches.size(); i++) {
    auto &match = matches[i];
    auto *match_array = allocate_array(match.size());
    v->item[i].type = T_ARRAY;
    v->item[i].u.arr = match_array;
    for (int j = 0; j < match.size(); j++) {
      match_array->item[j] = match[j];
    }
  }

  push_refed_array(v);
}
