#include "base/std.h"

#include "vm/internal/base/apply_cache.h"

#include <algorithm>

#include "vm/internal/base/program.h"

// TODO: move this to somewhere else.
static struct program_t *find_function_by_name2(struct program_t *, const char **, int *, int *,
                                                int *, int *);

#ifdef CACHE_STATS
unsigned int apply_low_call_others = 0;
unsigned int apply_low_cache_hits = 0;
unsigned int apply_low_slots_used = 0;
unsigned int apply_low_collisions = 0;
#endif

// TODO: this should be changed to allocated on heap.
// default initialized to 0
static cache_entry_t cache[APPLY_CACHE_SIZE];

/* Look up a entry for given fun/ob, user must validate
 * the entry content before use */
cache_entry_t *apply_cache_get_entry(const char *fun, const program_t *prog) {
  /* Search in cache for this function. */
  uint64_t ix; /* The cache index */
  std::hash<void *> hash_fn;
  ix = hash_fn((void *)fun);
  ix ^= hash_fn((void *)prog);
  return &cache[ix % (sizeof(cache) / sizeof(cache_entry_t))];
}

/* Erase the current entry. */
void apply_cache_clear_entry(cache_entry_t *entry) {
#ifdef CACHE_STATS
  if (!entry->funp) {
    apply_low_slots_used++;
  } else {
    apply_low_collisions++;
  }
#endif
  if (entry->oprogp) {
    free_prog(&entry->oprogp);
    entry->oprogp = 0;
  }
  if (entry->progp) {
    free_prog(&entry->progp);
    entry->progp = 0;
  } else {
    if (entry->funp) {
      free_string((char *)entry->funp);
      entry->funp = 0;
    }
  }
}

void apply_cache_save_entry(cache_entry_t *entry, program_t *target_prog, const char *sfun,
                            const char *fun, program_t *prog, int findex, int fio, int vio) {
  entry->oprogp = prog;
  reference_prog(entry->oprogp, "apply_low() cache oprogp [miss]");

  entry->progp = target_prog;
  entry->function_index_offset = fio;
  entry->variable_index_offset = vio;

  if (entry->progp) {
    reference_prog(entry->progp, "apply_low() cache progp [miss]");
    entry->funp = &target_prog->function_table[findex];
  } else {
    if (sfun) {
      ref_string(sfun);
      entry->funp = (function_t *)sfun;
    } else {
      entry->funp = (function_t *)make_shared_string(fun);
    }
  }
}

// Lookup the program, recursively check inherited object if needed,
// If program is not defined, return nullptr.
cache_entry_t *apply_cache_lookup(const char *fun, program_t *prog) {
#ifdef CACHE_STATS
  apply_low_call_others++;
#endif
  auto entry = apply_cache_get_entry(fun, prog);

  if (entry->oprogp == prog &&                                    /* prog must match */
      (entry->progp ? (strcmp(entry->funp->funcname, fun) == 0) : /* function name must match */
           strcmp((char *)entry->funp, fun) == 0)) {
#ifdef CACHE_STATS
    apply_low_cache_hits++;
#endif
  } else { /* not found in cache, search the function. */
    /* 1) Clean current entry */
    apply_cache_clear_entry(entry);

    /* 2) Search for the function */
    int findex = 0, runtime_index = 0, fio = 0, vio = 0;
    const char *sfun;
    sfun = fun;
    auto target_prog = find_function_by_name2(prog, &sfun, &findex, &runtime_index, &fio, &vio);

    /* 3) Save result into cache */
    // TODO: fix this idiotic sfun/fun.
    apply_cache_save_entry(entry, target_prog, sfun, fun, prog, findex, fio, vio);
  } /* search in cache */

  return entry;
}

static program_t *ffbn_recurse2(program_t *prog, const char *name, int *indexp, int *runtime_index,
                                int *fio, int *vio) {
  int high = prog->num_functions_defined - 1;
  int low = 0, mid;
  int ri;
  char *p;

  /* Search our function table */
  while (high >= low) {
    mid = (high + low) >> 1;
    p = prog->function_table[mid].funcname;
    if (name < p) {
      high = mid - 1;
    } else if (name > p) {
      low = mid + 1;
    } else {
      ri = mid + prog->last_inherited;

      if (prog->function_flags[ri] & (FUNC_UNDEFINED | FUNC_PROTOTYPE)) {
        return 0;
      }

      *indexp = mid;
      *runtime_index = ri;
      *fio = *vio = 0;
      return prog;
    }
  }

  /* Search inherited function tables */
  mid = prog->num_inherited;
  while (mid--) {
    program_t *ret = ffbn_recurse2(prog->inherit[mid].prog, name, indexp, runtime_index, fio, vio);
    if (ret) {
      *runtime_index += prog->inherit[mid].function_index_offset;
      *fio += prog->inherit[mid].function_index_offset;
      *vio += prog->inherit[mid].variable_index_offset;
      return ret;
    }
  }
  return 0;
}

static struct program_t *find_function_by_name2(program_t *prog, const char **name, int *indexp,
                                                int *runtime_index, int *fio, int *vio) {
  // all function name is shared string, if string is not a shared string,
  // it can not be a defined function.
  if (!(*name = findstring(*name))) {
    return 0;
  }
  return ffbn_recurse2(prog, *name, indexp, runtime_index, fio, vio);
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_apply_low_cache() {
  int i;
  for (i = 0; i < APPLY_CACHE_SIZE; i++) {
    if (cache[i].funp && !cache[i].progp) {
      EXTRA_REF(BLOCK((char *)cache[i].funp))++;
    }
    if (cache[i].oprogp) {
      cache[i].oprogp->extra_ref++;
    }
    if (cache[i].progp) {
      cache[i].progp->extra_ref++;
    }
  }
}
#endif
