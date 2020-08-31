#include "base/internal/stralloc.h"

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <climits>

#include "base/internal/debugmalloc.h"
#include "base/internal/hash.h"
#include "base/internal/log.h"
#include "base/internal/outbuf.h"
#include "base/internal/rc.h"

/* used temporarily by SVALUE_STRLEN() */
unsigned int svalue_strlen_size;

#ifdef NOISY_DEBUG
void bp(void) {}
#endif
/*
   this code is not the same as the original code.  I cleaned it up to
   use structs to: 1) make it easier to check the driver for memory leaks
   (using MallocDebug on a NeXT) and 2) because it looks cleaner this way.
   --Truilkan@TMI 92/04/19

   modified to make calls to strlen() unnecessary and to remove superfluous
   calls to findblock().  -- Truilkan@TMI, 1992/08/05
*/

/*
 * stralloc.c - string management.
 *
 * All strings are stored in an extensible hash table, with reference counts.
 * free_string decreases the reference count; if it gets to zero, the string
 * will be deallocated.  add_string increases the ref count if it finds a
 * matching string, or allocates it if it cant.  There is no way to allocate
 * a string of a particular size to fill later (hash wont work!), so you'll
 * have to copy things from a static (or malloced and later freed) buffer -
 * that is, if you want to avoid space leaks...
 *
 * Current overhead:
 *      sizeof(block_t) per string (string pointer, next pointer, and a short
 *  for refs). Strings are nearly all fairly short, so this is a significant
 *  overhead - there is also the 4 byte malloc overhead and the fact that
 *  malloc generally allocates blocks which are a power of 2 (should write my
 *      own best-fit malloc specialised to strings); then again, GNU malloc
 *      is bug free...
 */

/*
 * there is also generic hash table management code, but strings can be shared
 * (that was the point of this code), will be unique in the table,
 * require a reference count, and are malloced, copied and freed at
 * will by the string manager.  Besides, I wrote this code first :-).
 * Look at htable.c for the other code.  It uses the Hash() function
 * defined here, and requires hashed objects to have a pointer to the
 * next element in the chain (which you specify when you call the functions).
 */

int num_distinct_strings = 0;
int bytes_distinct_strings = 0;
int overhead_bytes = 0;
int allocd_strings = 0;
int allocd_bytes = 0;
int search_len = 0;
int num_str_searches = 0;

#define StrHash(s) (whashstr((s)) & (htable_size_minus_one))

#define hfindblock(s, h) sfindblock(s, h = StrHash(s))
#define findblock(s) sfindblock(s, StrHash(s))

static block_t *sfindblock(const char * /*s*/, int /*h*/);

/*
 * hash table - list of pointers to heads of string chains.
 * Each string in chain has a pointer to the next string and a
 * reference count (char *, int) stored just before the start of the string.
 * HTABLE_SIZE is in config.h, and should be a prime, probably between
 * 1000 and 5000.
 */

static block_t **base_table = (block_t **)nullptr;
static int htable_size;
static int htable_size_minus_one;

static block_t *alloc_new_string(const char * /*string*/, int /*h*/, const char * /*why*/);

void init_strings() {
  int x, y;

  /* ensure that htable size is a power of 2 */
  y = CONFIG_INT(__SHARED_STRING_HASH_TABLE_SIZE__);
  for (htable_size = 1; htable_size < y; htable_size *= 2) {
    ;
  }
  htable_size_minus_one = htable_size - 1;
  base_table = reinterpret_cast<block_t **>(
      DCALLOC(htable_size, sizeof(block_t *), TAG_STR_TBL, "init_strings"));
  overhead_bytes += (sizeof(block_t *) * htable_size);

  for (x = 0; x < htable_size; x++) {
    base_table[x] = nullptr;
  }
}

/*
 * Looks for a string in the table.  If it finds it, returns a pointer to
 * the start of the string part, and moves the entry for the string to
 * the head of the pointer chain.  One thing (blech!) - puts the previous
 * pointer on the hash chain into fs_prev.
 */

static block_t *sfindblock(const char *s, int h) {
  block_t *curr, *prev;

  curr = base_table[h];
  prev = nullptr;
  num_str_searches++;

  while (curr) {
    search_len++;

    if (*(STRING(curr)) == *s && !strcmp(STRING(curr), s)) { /* found it */
      if (prev) {                                            /* not at head of list */
        NEXT(prev) = NEXT(curr);
        NEXT(curr) = base_table[h];
        base_table[h] = curr;
      }
      return (curr); /* pointer to string */
    }
    prev = curr;
    curr = NEXT(curr);
  }
  return ((block_t *)nullptr); /* not found */
}

char *findstring(const char *s) {
  block_t *b;

  if ((b = findblock(s))) {
    return STRING(b);
  } else {
    return (nullptr);
  }
}

/* alloc_new_string: Make a space for a string.  */

static block_t *alloc_new_string(const char *string, int h, const char *why) {
  auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);

  block_t *b;
  int len = strlen(string);
  int size;
  int cut = 0;
  if (len > max_string_length) {
    len = max_string_length;
    cut = 1;
  }
  size = sizeof(block_t) + len + 1;
  b = reinterpret_cast<block_t *>(DMALLOC(size, TAG_SHARED_STRING, why));
  strncpy(STRING(b), string, len);
  STRING(b)
  [len] = '\0'; /* strncpy doesn't put on \0 if 'from' too
                 * long */
  if (cut) {
    h = whashstr(STRING(b)) & htable_size_minus_one;
  }
  SIZE(b) = (len > UINT_MAX ? UINT_MAX : len);
  REFS(b) = 1;
  NEXT(b) = base_table[h];
  HASH(b) = h;
  base_table[h] = b;
  ADD_NEW_STRING(SIZE(b), sizeof(block_t));
  ADD_STRING(SIZE(b));
  return (b);
}

char *make_shared_string(const char *str) {
  block_t *b;
  int h;

  b = hfindblock(str, h); /* hfindblock macro sets h = StrHash(s) */
  if (!b) {
    b = alloc_new_string(str, h, "make_shared_string");
  } else {
    if (REFS(b)) {
      REFS(b)++;
    }
    ADD_STRING(SIZE(b));
  }
  NDBG(b);
  return (STRING(b));
}

/*
   ref_string: Fatal to call this function on a string that isn't shared.
*/

const char *ref_string(const char *str) {
  block_t *b;

  b = BLOCK(str);
  DEBUG_CHECK1(b != findblock(str), "stralloc.c: called ref_string on non-shared string: %s.\n",
               str);
  if (REFS(b)) {
    REFS(b)++;
  }
  NDBG(b);
  ADD_STRING(SIZE(b));
  return str;
}

/* free_string: fatal to call free_string on a non-shared string */
/*
 * free_string - reduce the ref count on a string.  Various sanity
 * checks applied.
 */

void free_string(const char *str) {
  block_t **prev, *b;
  int h;

  b = BLOCK(str);
  DEBUG_CHECK1(b != findblock(str), "stralloc.c: free_string called on non-shared string: %s.\n",
               str);

  /*
   * if a string has been ref'd USHRT_MAX times then we assume that its used
   * often enough to justify never freeing it.
   */
  if (!REFS(b)) {
    return;
  }

  REFS(b)--;
  SUB_STRING(SIZE(b));

  NDBG(b);
  if (REFS(b) > 0) {
    return;
  }

  // h = StrHash(str);
  h = HASH(BLOCK(str));
  prev = base_table + h;
  while ((b = *prev)) {
    if (STRING(b) == str) {
      *prev = NEXT(b);
      break;
    }
    prev = &(NEXT(b));
  }

  DEBUG_CHECK1(!b, "free_string: not found in string table! (\"%s\")\n", str);

  SUB_NEW_STRING(SIZE(b), sizeof(block_t));
  FREE(b);
  CHECK_STRING_STATS;
}

void deallocate_string(char *str) {
  int h;
  block_t *b, **prev;

  // h = StrHash(str);
  h = HASH(BLOCK(str));
  prev = base_table + h;
  while ((b = *prev)) {
    if (STRING(b) == str) {
      *prev = NEXT(b);
      break;
    }
    prev = &(NEXT(b));
  }
  DEBUG_CHECK1(!b, "stralloc.c: deallocate_string called on non-shared string: %s.\n", str);
  // printf("freeing string: %s\n", str);
  FREE(b);
}

int add_string_status(outbuffer_t *out, int verbose) {
  if (verbose == 1) {
    outbuf_add(out, "All strings:\n");
    outbuf_add(out, "-------------------------\t Strings    Bytes\n");
  }
  if (verbose != -1) {
    outbuf_addv(out, "All strings:\t\t\t%8d %8d + %d overhead\n", num_distinct_strings,
                bytes_distinct_strings, overhead_bytes);
  }
  if (verbose == 1) {
    outbuf_addv(out, "Total asked for\t\t\t%8d %8d\n", allocd_strings, allocd_bytes);
    outbuf_addv(out, "Space actually required/total string bytes %d%%\n",
                (bytes_distinct_strings + overhead_bytes) * 100 / allocd_bytes);
    outbuf_addv(out, "Searches: %d    Average search length: %6.3f\n", num_str_searches,
                static_cast<double>(search_len) / num_str_searches);
  }
  return (bytes_distinct_strings + overhead_bytes);
}

#ifdef DEBUGMALLOC_EXTENSIONS
#define DME 0,
#else
#define DME
#endif

/* This stuff needs a bit more work, otherwise FREE_MSTR() will crash on this
malloc_block_t the_null_string_blocks[2] = { { DME 0, 1 }, { DME 0, 0 } };

char *the_null_string = (char *)&the_null_string_blocks[1];
*/

#ifdef DEBUGMALLOC
char *int_new_string(unsigned int size, const char *tag)
#else
char *int_new_string(unsigned int size)
#endif
{
  malloc_block_t *mbt;

#if 0
  if (!size) {
    the_null_string_blocks[0].ref++;
    ADD_NEW_STRING(0, sizeof(malloc_block_t));
    return the_null_string;
  }
#endif

  mbt = reinterpret_cast<malloc_block_t *>(
      DMALLOC(size + sizeof(malloc_block_t) + 1, TAG_MALLOC_STRING, tag));
  if (size < UINT_MAX) {
    mbt->size = size;
    ADD_NEW_STRING(size, sizeof(malloc_block_t));
  } else {
    mbt->size = UINT_MAX;
    ADD_NEW_STRING(UINT_MAX, sizeof(malloc_block_t));
  }
  mbt->ref = 1;
  ADD_STRING(mbt->size);
  CHECK_STRING_STATS;
  return reinterpret_cast<char *>(mbt + 1);
}

char *extend_string(const char *str, int len) {
  malloc_block_t *mbt;
  int oldsize = MSTR_SIZE(str);

  mbt = reinterpret_cast<malloc_block_t *>(DREALLOC(
      MSTR_BLOCK(str), len + sizeof(malloc_block_t) + 1, TAG_MALLOC_STRING, "extend_string"));
  if (len < UINT_MAX) {
    mbt->size = len;
  } else {
    mbt->size = UINT_MAX;
  }
  ADD_STRING_SIZE(mbt->size - oldsize);
  CHECK_STRING_STATS;

  return reinterpret_cast<char *>(mbt + 1);
}

#ifdef DEBUGMALLOC
char *int_alloc_cstring(const char *str, const char *tag)
#else
char *int_alloc_cstring(const char *str)
#endif
{
  char *ret;

  ret = reinterpret_cast<char *>(DMALLOC(strlen(str) + 1, TAG_STRING, tag));
  strcpy(ret, str);
  return ret;
}

#ifdef DEBUGMALLOC
char *int_string_copy(const char *const str, const char *desc)
#else
char *int_string_copy(const char *const str)
#endif
{
  // TODO: probabaly not a good idea to have it here.
  auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);

  char *p;
  int len;

  DEBUG_CHECK(!str, "Null string passed to string_copy.\n");
  len = strlen(str);
  if (len > max_string_length) {
    len = max_string_length;
    p = new_string(len, desc);
    (void)strncpy(p, str, len);
    p[len] = '\0';
  } else {
    p = new_string(len, desc);
    (void)strncpy(p, str, len + 1);
  }
  return p;
}

#ifdef DEBUGMALLOC
char *int_string_unlink(const char *str, const char *desc)
#else
char *int_string_unlink(const char *str)
#endif
{
  malloc_block_t *mbt, *newmbt;

  mbt = ((malloc_block_t *)str) - 1;
  mbt->ref--;

  if (mbt->size == USHRT_MAX) {
    int l = strlen(str + USHRT_MAX) + USHRT_MAX; /* ouch */

    newmbt = reinterpret_cast<malloc_block_t *>(
        DMALLOC(l + sizeof(malloc_block_t) + 1, TAG_MALLOC_STRING, desc));
    memcpy(reinterpret_cast<char *>(newmbt + 1), reinterpret_cast<char *>(mbt + 1), l + 1);
    newmbt->size = USHRT_MAX;
    ADD_NEW_STRING(USHRT_MAX, sizeof(malloc_block_t));
  } else {
    newmbt = reinterpret_cast<malloc_block_t *>(
        DMALLOC(mbt->size + sizeof(malloc_block_t) + 1, TAG_MALLOC_STRING, desc));
    memcpy(reinterpret_cast<char *>(newmbt + 1), reinterpret_cast<char *>(mbt + 1), mbt->size + 1);
    newmbt->size = mbt->size;
    ADD_NEW_STRING(mbt->size, sizeof(malloc_block_t));
  }
  newmbt->ref = 1;
  CHECK_STRING_STATS;

  return reinterpret_cast<char *>(newmbt + 1);
}
