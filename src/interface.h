#include "lpc_to_c.h"

#ifdef LPC_TO_C
typedef struct {
    char *fname;
    void (**jump_table) ();
    string_switch_entry_t **string_switch_tables;
} interface_t;

extern interface_t *interface[];
#endif
