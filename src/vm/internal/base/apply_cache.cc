#include "base/std.h"

#include "vm/internal/base/apply_cache.h"

#include <algorithm>

#include "base/internal/tracing.h"
#include "vm/internal/base/program.h"

static inline void fill_lookup_table(program_t *prog);

lookup_entry_s apply_cache_lookup(const char *funcname, program_t *prog) {
  ScopedTracer _tracer("Apply Cache Lookup", EventCategory::APPLY_CACHE, {{"name", funcname}});

  // All function names are shared string.
  auto key = (intptr_t)(findstring(funcname));
  if (key == 0) {
    return lookup_entry_s{nullptr};
  }

  auto table = prog->apply_lookup_table;
  if (table == nullptr) {
    fill_lookup_table(prog);
    table = prog->apply_lookup_table;
  }

  apply_cache_lookups++;

  auto pos = table->find(key);
  if (pos != table->end()) {
    apply_cache_hits++;
    return pos->second;
  } else {
    return lookup_entry_s{nullptr};
  }
}

static inline void fill_lookup_table_recurse(void *table, program_t *prog, uint16_t fio,
                                             uint16_t vio) {
  // add all defined functions
  for (int i = 0; i < prog->num_functions_defined; i++) {
    auto runtime_index = i + prog->last_inherited;
    if (prog->function_flags[runtime_index] & (FUNC_UNDEFINED | FUNC_PROTOTYPE)) {
      continue;
    }

    auto real_table = decltype(prog->apply_lookup_table)(table);
    auto key = (intptr_t)(prog->function_table[i].funcname);
    lookup_entry_s entry = {nullptr};
    entry.progp = prog;
    entry.funp = &(prog->function_table[i]);
    entry.function_index_offset = fio;
    entry.variable_index_offset = vio;

    real_table->insert({key, entry});
  }

  // add inherited functions (must go backwards)
  int i = prog->num_inherited;
  while (i--) {
    auto inherit = prog->inherit[i];
    fill_lookup_table_recurse(table, inherit.prog, fio + inherit.function_index_offset,
                              vio + inherit.variable_index_offset);
  }
}

static inline void fill_lookup_table(program_t *prog) {
  auto table = prog->apply_lookup_table;
  table = new std::remove_pointer<decltype(table)>::type();
  prog->apply_lookup_table = table;

  fill_lookup_table_recurse(table, prog, 0, 0);

  apply_cache_items += table->size();
}
