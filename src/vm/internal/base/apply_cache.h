#ifndef LPC_APPLY_CACHE_H_
#define LPC_APPLY_CACHE_H_

struct cache_entry_t {
  struct program_t *oprogp;
  struct program_t *progp;
  struct function_t *funp;
  unsigned short function_index_offset;
  unsigned short variable_index_offset;
};

cache_entry_t *apply_cache_lookup(const char *, struct program_t *);

#endif /* LPC_APPLY_CACHE_H_ */
