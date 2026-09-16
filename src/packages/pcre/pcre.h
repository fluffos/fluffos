/* OVECCOUNT is the return vector of matches.
   ovector[0] = start of entire match
   ovector[1] = end of entire match
   ovector[2] = start of first capture group
   ovector[3] = end of first capture group
   ovector[4] = start of second capture group
     etc

The maximum number of capturing groups is:
   (n+1)*3
*/
#ifndef PACKAGS_PCRE_H
#define PACKAGS_PCRE_H

#define PCRE_CACHE_SIZE 256

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

typedef struct {
  pcre2_code* re;
  char error[256];
  const char* pattern;
  const char* subject;
  PCRE2_SPTR name_table;
  size_t s_length;
  int erroffset;
  int find_all;
  uint32_t namecount;
  uint32_t name_entry_size;
  int compile_flags;
  int exec_flags;
  int* ovector;
  int ovecsize;
  int rc;
  /* EXTRA */
} pcre_t;

struct pcre_cache_bucket_t {
  pcre2_code* compiled_pattern;  // value1
  const char* pattern;           // key
  int compile_flags;             // compile options used
  int size;                      // size in bytes
  struct pcre_cache_bucket_t* next;
};

struct pcre_cache_t {
  struct pcre_cache_bucket_t* buckets[PCRE_CACHE_SIZE];
};

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_pcre_cache();
#endif

#endif
