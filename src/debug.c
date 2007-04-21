#include "std.h"
#include "lpc_incl.h"

#ifdef DEBUG_MACRO

mapping_t *debug_levels (void);

typedef struct {
    char *name;
    int bit;
} debug_t;

#define E(x) { #x, DBG_##x }

debug_t levels[] = {
    E(call_out),
    E(d_flag),
    E(connections),
    E(mapping),
    E(sockets),
    E(comp_func_tab),
    E(LPC),
    E(LPC_line)
};

#define NELEM(x) (sizeof(x)/sizeof(x[0]))

mapping_t *debug_levels() {
    int dl = debug_level;
    mapping_t *ret = allocate_mapping(10);
    int i;
    
    for (i = 0; i < NELEM(levels); i++) {
	add_mapping_pair(ret, levels[i].name, dl & levels[i].bit);
	dl &= ~levels[i].bit;
    }

    if (dl)
	add_mapping_pair(ret, "unknown", dl);
    
    return ret;
}

void debug_level_set (char * level) {
    int i;
    
    for (i = 0; i < NELEM(levels); i++) {
	if (strcmp(level, levels[i].name) == 0) {
	    debug_level |= levels[i].bit;
	    return;
	}
    }
}

void debug_level_clear (char * level) {
    int i;
    
    for (i = 0; i < NELEM(levels); i++) {
	if (strcmp(level, levels[i].name) == 0) {
	    debug_level &= ~levels[i].bit;
	    return;
	}
    }
}

#endif
