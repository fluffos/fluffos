#ifndef SIMUL_EFUN_H
#define SIMUL_EFUN_H

/*
 * simul_efun.c
 */
typedef struct {
    compiler_function_t *func;
    int index;
} simul_info_t;

extern object_t *simul_efun_ob;
extern simul_info_t *simuls;

void init_simul_efun PROT((char *));
void set_simul_efun PROT((object_t *));
int find_simul_efun PROT((char *));

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_simuls PROT((void));
#endif

#endif
