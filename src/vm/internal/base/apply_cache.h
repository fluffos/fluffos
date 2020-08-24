#ifndef LPC_APPLY_CACHE_H_
#define LPC_APPLY_CACHE_H_

#include "vm/internal/base/program.h"

lookup_entry_s apply_cache_lookup(const char *funcname, struct program_t *prog);

#endif /* LPC_APPLY_CACHE_H_ */
