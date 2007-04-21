#ifndef MASTER_H
#define MASTER_H

#include "lpc_incl.h"

/* for apply_master_ob */
#define MASTER_APPROVED(x) (((x)==(svalue_t *)-1) || ((x) && (((x)->type != T_NUMBER) || (x)->u.number))) 

extern object_t *master_ob;

svalue_t *apply_master_ob (int, int);
svalue_t *safe_apply_master_ob (int, int);
void init_master (void);
void set_master (object_t *);

#endif
