#ifndef COMPILE_FILE_H
#define COMPILE_FILE_H

void init_lpc_to_c PROT((void));
int generate_source PROT((svalue_t *, char *));
void link_jump_table PROT((program_t *, void **));

#endif

