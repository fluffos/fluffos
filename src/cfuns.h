#ifndef CFUNS_H
#define CFUNS_H

#ifdef LPC_TO_C
void fix_switches PROT((string_switch_entry_t **));

extern int lpc_int;
extern svalue_t *lpc_svp;
extern array_t *lpc_arr;
extern mapping_t *lpc_map;
#endif

#endif

