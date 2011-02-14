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
#ifndef PCRE_EFUNS
#define PCRE_EFUNS
#define PCRE_CACHE_SIZE 256

typedef struct {
   pcre *re;
   const char *error;
   svalue_t pattern;
   const char *subject;
   unsigned char *name_table;
   size_t s_length;
   int erroffset;
   int find_all;
   int namecount;
   int name_entry_size;
   int *ovector;
   int ovecsize;
   int rc;
/* EXTRA */
} pcre_t;

struct pcre_cache_bucket_t {
   pcre *compiled_pattern; // value1
   svalue_t pattern;    // key
   int size;               // size in bytes
   struct pcre_cache_bucket_t *next;
};

struct pcre_cache_t {
   struct pcre_cache_bucket_t *buckets[PCRE_CACHE_SIZE];
};


#endif

