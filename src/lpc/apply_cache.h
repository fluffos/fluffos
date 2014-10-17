#ifndef LPC_APPLY_CACHE_H_
#define LPC_APPLY_CACHE_H_

typedef struct program_s program_t;
struct function_t;
typedef struct object_s object_t;

struct cache_entry_t {
  program_t *oprogp;
  program_t *progp;
  function_t *funp;
  unsigned short function_index_offset;
  unsigned short variable_index_offset;
};

cache_entry_t *apply_cache_lookup(const char *, object_t *);

#endif /* LPC_APPLY_CACHE_H_ */
