/* OVECCOUNT is the return vector of matches (PCRE2: two slots per group).
   ovector[0] = start of entire match
   ovector[1] = end of entire match
   ovector[2] = start of first capture group
   ovector[3] = end of first capture group
   ovector[4] = start of second capture group
     etc
*/
#ifndef PACKAGS_PCRE_H
#define PACKAGS_PCRE_H

#define PCRE_CACHE_SIZE 256

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

typedef struct {
  pcre2_code* re;
  pcre2_match_context* mcontext;
  char error[256];
  const char* pattern;
  const char* subject;
  PCRE2_SPTR name_table;
  size_t s_length;
  int erroffset;
  int find_all;
  uint32_t namecount;
  uint32_t name_entry_size;
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
  int* ovector;
  int ovecsize;
  int rc;
} pcre_t;

struct pcre_cache_bucket_t {
  pcre2_code* compiled_pattern;  // value1
  const char* pattern;           // key
  uint32_t compile_flags;
  uint32_t extra_options;
  uint32_t newline;
  uint32_t bsr;
  int no_jit;
  int size;  // size in bytes
  struct pcre_cache_bucket_t* next;
};

struct pcre_cache_t {
  struct pcre_cache_bucket_t* buckets[PCRE_CACHE_SIZE];
};

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_pcre_cache();
#endif

#endif
