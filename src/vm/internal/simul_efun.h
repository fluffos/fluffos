#ifndef SIMUL_EFUN_H
#define SIMUL_EFUN_H

/*
 * simul_efun.c
 */

extern struct object_t *simul_efun_ob;
extern struct function_lookup_info_t *simuls;

void init_simul_efun(const char *);
void set_simul_efun(struct object_t *);

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_simuls(void);
#endif

void call_simul_efun(unsigned short, int);

#endif
