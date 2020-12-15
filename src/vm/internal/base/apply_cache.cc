#include "base/std.h"

#include "vm/internal/base/apply_cache.h"

#include <algorithm>
#include <memory>

#include "base/internal/tracing.h"
#include "vm/internal/base/program.h"

static inline void fill_lookup_table(program_t *prog);

lookup_entry_s apply_cache_lookup(const char *funcname, program_t *prog) {
  ScopedTracer _tracer("Apply Cache Lookup", EventCategory::APPLY_CACHE, json{{"name", funcname}});

  // All function names are shared string.
  auto key = (intptr_t)(findstring(funcname));
  if (key == 0) {
    return lookup_entry_s{nullptr};
  }

  if (prog->apply_lookup_table == nullptr) {
    fill_lookup_table(prog);
  }

  apply_cache_lookups++;

  auto pos = prog->apply_lookup_table->find(key);
  if (pos != prog->apply_lookup_table->end()) {
    apply_cache_hits++;
    return pos->second;
  } else {
    return lookup_entry_s{nullptr};
  }
}

static inline void fill_lookup_table_recurse(
    std::unique_ptr<program_t::apply_lookup_table_type> &table, program_t *prog, uint16_t fio,
    uint16_t vio) {
  // add all defined functions
  for (int i = 0; i < prog->num_functions_defined; i++) {
    auto runtime_index = i + prog->last_inherited;
    if (prog->function_flags[runtime_index] & (FUNC_UNDEFINED | FUNC_PROTOTYPE)) {
      continue;
    }

    auto key = (intptr_t)(prog->function_table[i].funcname);
    lookup_entry_s entry = {nullptr};
    entry.progp = prog;
    entry.funp = &(prog->function_table[i]);
    entry.function_index_offset = fio;
    entry.variable_index_offset = vio;

    table->insert({key, entry});
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
  prog->apply_lookup_table = std::make_unique<program_t::apply_lookup_table_type>();
  fill_lookup_table_recurse(prog->apply_lookup_table, prog, 0, 0);

  apply_cache_items += prog->apply_lookup_table->size();
}
